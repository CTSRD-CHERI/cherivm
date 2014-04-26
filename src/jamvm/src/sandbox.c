#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#include "jam.h"
#include "symbol.h"

#ifdef JNI_CHERI

#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>

#include <cheri/sandbox.h>
#include <cheri/cheri_class.h>
#include <cheri/cheri_enter.h>
#include <cheri/cheri_system.h>
#include <cheri/cheri_invoke.h>

#include "sandbox.h"
#include "sandbox_shared.h"

extern JNIEnv globalJNIEnv;
static pClass class_String;

struct sandbox_object {
	struct sandbox_class	*sbo_sandbox_classp;
	void			*sbo_mem;
	register_t		 sbo_sandboxlen;
	register_t		 sbo_heapbase;
	register_t		 sbo_heaplen;
	struct cheri_object	 sbo_cheri_object;
	struct cheri_object	 sbo_cheri_system_object;
	struct sandbox_object_stat	*sbo_sandbox_object_statp;
};

typedef struct cherijni_sandbox {
        struct sandbox_class    *classp;
        struct sandbox_object   *objectp;
} cherijniSandbox;

typedef __capability void* (*fn_jni_cap)(struct cheri_object, u_int, register_t, register_t, register_t, register_t, register_t, register_t, register_t, __capability void *, __capability void *, __capability void *, __capability void *,   __capability void *,__capability void *, __capability void *, __capability void *) __attribute__((cheri_ccall));

#define DEFINE_SEAL_VARS(NAME)                                \
        static uintptr_t          type_##NAME;                      \
        static __capability void *sealcap_##NAME;                   \

#define cap_seal(NAME, data)             cherijni_seal(data, sealcap_##NAME)
#define cap_unseal(TYPE, NAME, datacap)  ((TYPE) cherijni_unseal(datacap, sealcap_##NAME))
#define INIT_SEAL(NAME) sealcap_##NAME = cheri_ptrtype(&type_##NAME, sizeof(uintptr_t), 0); // sets PERMIT_SEAL

DEFINE_SEAL_VARS(JavaObject)
DEFINE_SEAL_VARS(Context)
DEFINE_SEAL_VARS(MethodID)
DEFINE_SEAL_VARS(FieldID)
DEFINE_SEAL_VARS(FILE)

#define RETURNTYPE_VOID   1
#define RETURNTYPE_SINGLE 2
#define RETURNTYPE_DOUBLE 3
#define RETURNTYPE_OBJECT 4

static inline __capability void *cherijni_seal(void *data, __capability void *sealcap) {
	if (data == NULL)
		return CNULL;
	else
		return cheri_sealdata(cheri_ptrperm(data, sizeof(uintptr_t), CHERI_PERM_LOAD), sealcap);
}

static inline void *cherijni_unseal(__capability void *objcap, __capability void *sealcap) {
	if (objcap == CNULL)
		return NULL;

	// is it a valid capability?
	if (!cheri_gettag(objcap)) {
		jam_printf("Warning: provided cap cannot be unsealed (tag not set)\n");
		return NULL;
	}

	// is it sealed?
	if (cheri_getunsealed(objcap)) {
		jam_printf("Warning: provided cap cannot be unsealed (not sealed)\n");
		return NULL;
	}

	// is it of the correct type?
	if (cheri_gettype(objcap) != cheri_gettype(sealcap)) {
		jam_printf("Warning: provided cap cannot be unsealed (wrong type)\n");
		return NULL;
	}

	// unseal, convert to a pointer, return
	return (void*) cheri_unseal(objcap, sealcap);
}

char *cherijni_libName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof(".cheri") + 1);

   sprintf(buff, "%s.cheri", name);
   return buff;
}

#define CInvoke_7_6(handle, op, a1, a2, a3, a4, a5, a6, a7, c5, c6, c7, c8, c9, c10)                         \
		sandbox_object_cinvoke ( \
				((cherijniSandbox *) handle)->objectp, op,                                           \
	            (register_t) a1, (register_t) a2, (register_t) a3, (register_t) a4,                          \
                (register_t) a5, (register_t) a6, (register_t) a7,                                           \
	            sandbox_object_getsystemobject(((cherijniSandbox *) handle)->objectp).co_codecap,    \
	            sandbox_object_getsystemobject(((cherijniSandbox *) handle)->objectp).co_datacap,    \
	            c5, c6, c7, c8, c9, c10)
#define CInvoke_1_1(handle, op, a1, c5)  CInvoke_7_6(handle, op, a1, 0, 0, 0, 0, 0, 0, c5, CNULL, CNULL, CNULL, CNULL, CNULL)
#define CInvoke_1_0(handle, op, a1)      CInvoke_1_1(handle, op, a1, CNULL)
#define CInvoke_0_1(handle, op, c5)      CInvoke_1_1(handle, op, 0, c5)
#define CInvoke_0_0(handle, op)          CInvoke_1_1(handle, op, 0, CNULL)

void *cherijni_open(char *path) {
	cherijniSandbox *sandbox = sysMalloc(sizeof(cherijniSandbox));

	/* Initialize the sandbox class and object */
	if (sandbox_class_new(path, DEFAULT_SANDBOX_MEM, &sandbox->classp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	if (sandbox_object_new(sandbox->classp, &sandbox->objectp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	return sandbox;
}

void cherijni_runTests(void *handle, pClass context) {
	__capability void *cJNIContext = cap_seal(Context, context);
	CInvoke_0_1(handle, CHERIJNI_METHOD_TEST, cJNIContext);
}

// TODO: make sure only one thread ever enters the sandbox !!!
// TODO: check the return value? -1 *may* mean that a trap happened inside the sandbox (will be replaced with signals)

void *cherijni_lookup(void *handle, char *methodName) {
	return (void*) CInvoke_0_1(handle, CHERIJNI_METHOD_LOOKUP, cap_string(methodName));
}

jint cherijni_callOnLoadUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved) {
	__capability void *cJvm = cheri_ptr(jvm, sizeof(JavaVM));
	return (jint) CInvoke_1_0(handle, CHERIJNI_METHOD_ONLOAD_ONUNLOAD, ptr);
}

#define arg_cap(cap, len, perm)            checkSandboxCapability_##perm(cap, len)
#define arg_ptr(cap, len, perm)            convertSandboxPointer(arg_cap(cap, len, perm))
#define arg_str(ptr, len, perm)            ((const char*) arg_ptr(ptr, len, perm))
#define arg_obj(cap)    cap_unseal(pObject, JavaObject, cap)
#define arg_class(cap)  checkIsClass(arg_obj(cap))
#define arg_file(cap)   cap_unseal(FILE*, FILE, cap)
#define return_obj(obj)    { *mem_output = cap_seal(JavaObject, obj); }
#define return_mid(field)  { (*mem_output) = cap_seal(MethodID, field); }
#define return_fid(field)  { (*mem_output) = cap_seal(FieldID, field); }
#define return_file(file)  { (*mem_output) = cap_seal(FILE, file); }
#define return_str(str)    { (*mem_output) = cap_string(str); }

static inline void copyToSandbox(const char *src, __capability char *dest, size_t len) {
	size_t i;
	for (i = 0; i < len; i++)
		dest[i] = src[i];
}

static inline __capability void *checkSandboxCapability(__capability void *cap, size_t min_length, register_t perm_mask) {
	if (!cheri_gettag(cap)) {
		jam_printf("Warning: sandbox provided an invalid capability\n");
		return CNULL;
	}

	if (!cheri_getunsealed(cap)) {
		jam_printf("Warning: sandbox provided a sealed pointer capability\n");
		return CNULL;
	}

	if (min_length > 0 && cheri_getlen(cap) < min_length) {
		jam_printf("Warning: sandbox provided a pointer capability which is too short\n");
		return CNULL;
	}

	register_t perm_cap = cheri_getperm(cap);
	if ((perm_cap & perm_mask) != perm_mask) {
		jam_printf("Warning: sandbox provided a pointer capability with insufficient permissions\n");
		return CNULL;
	}

	return cap;
}

static inline void *convertSandboxPointer(__capability void *cap) {
	// NULL capability?
	if (cap == CNULL)
		return NULL;

	return (void*) cap;
}

static inline __capability void *checkSandboxCapability_wc(__capability void *cap, size_t min_length) {
	return checkSandboxCapability(cap, min_length, CHERI_PERM_STORE_CAP);
}

static inline __capability void *checkSandboxCapability_r(__capability void *cap, size_t min_length) {
	return checkSandboxCapability(cap, min_length, CHERI_PERM_LOAD);
}

static inline __capability void *checkSandboxCapability_w(__capability void *cap, size_t min_length) {
	return checkSandboxCapability(cap, min_length, CHERI_PERM_STORE);
}

static inline pClass checkIsClass(pObject obj) {
	if (obj == NULL)
		return NULL;
	else if (IS_CLASS(obj))
		return (pClass) obj;
	else {
		jam_printf("Warning: expected a class object from sandbox\n");
		return NULL;
	}
}

static inline int checkIsValidString(pObject obj) {
	return (obj != NULL) && (obj->class == class_String);
}

/*
 * XXX: HACKISH!!! Bypassing cheri_sandbox_cinvoke
 */
static __capability void *invoke_returnCap(void *handle, void *native_func, __capability void *cap_signature, __capability void *cap_this, register_t args_prim[], __capability void *args_cap[]) {
	fn_jni_cap func = (fn_jni_cap) cheri_invoke;
	cherijniSandbox *sandbox = (cherijniSandbox*) handle;
	return (func)(
			sandbox->objectp->sbo_cheri_object,
			CHERIJNI_METHOD_RUN, native_func,
			args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5],
			sandbox_object_getsystemobject(sandbox->objectp).co_codecap,
			sandbox_object_getsystemobject(sandbox->objectp).co_datacap,
            cap_signature, cap_this,
            args_cap[0], args_cap[1], args_cap[2], args_cap[3]);
}

static register_t invoke_returnPrim(void *handle, void *native_func, __capability void *cap_signature, __capability void *cap_this, register_t args_prim[], __capability void *args_cap[]) {
	return CInvoke_7_6(handle, CHERIJNI_METHOD_RUN, native_func,
	                   args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5],
	                   cap_signature, cap_this,
	                   args_cap[0], args_cap[1], args_cap[2], args_cap[3]);
}

static void fillArguments(char *sig, uintptr_t *_ostack, register_t *_pPrimitiveArgs, __capability void **_pObjectArgs) {
	scanSignature(sig,
		/* single primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); },
		/* double primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); _ostack++; },
		/* objects           */ { *(_pObjectArgs++) = cap_seal(JavaObject, (pObject) *(_ostack++)); },
		/* return values     */ { }, { }, { }, { });
}

uintptr_t *cherijni_callMethod(void* handle, void *native_func, pClass class, char *sig, uintptr_t *ostack) {
	__capability void *cap_this;
	uintptr_t *_ostack = ostack;
	size_t cPrimitiveArgs = 0, cObjectArgs = 0, returnType;
	register_t args_prim [6] = {0, 0, 0, 0, 0, 0};
	__capability void *args_cap [4] = {CNULL, CNULL, CNULL, CNULL};

	/* Count the arguments */
	scanSignature(sig,
		/* single primitives */ { (cPrimitiveArgs++); },
		/* double primitives */ { (cPrimitiveArgs++); },
		/* objects           */ { (cObjectArgs++); },
		/* return values     */ { returnType = RETURNTYPE_VOID; },
		                        { returnType = RETURNTYPE_SINGLE; },
		                        { returnType = RETURNTYPE_DOUBLE; },
		                        { returnType = RETURNTYPE_OBJECT; });

	// TODO: check number of arguments

	/* Prepare arguments */
	if (class == NULL) cap_this = cap_seal(JavaObject, (pObject) *(_ostack++));
	else               cap_this = cap_seal(JavaObject, class);
	fillArguments(sig, _ostack, args_prim, args_cap);

	/* Invoke JNI method */

	if (returnType == RETURNTYPE_OBJECT) {
		__capability void *cap_result = invoke_returnCap(handle, native_func, cap_string(sig), cap_this, args_prim, args_cap);
		pObject result_obj = arg_obj(cap_result);
		*(ostack++) = (uintptr_t) result_obj;
	} else {
		register_t result = invoke_returnPrim(handle, native_func, cap_string(sig), cap_this, args_prim, args_cap);
		if (returnType == RETURNTYPE_SINGLE)
			*(ostack++) = result;
		else if (returnType == RETURNTYPE_DOUBLE) {
			*(ostack++) = result;
			ostack++;
		}
	}

	return ostack;
}

#define JNI_FUNCTION(NAME) \
	static register_t JNI_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *cap_output, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4) { \
	JNIEnv *env = &globalJNIEnv; \
	__capability void **mem_output = arg_ptr(cap_output, sizeof(__capability void*), wc);
/*
 	__capability void *cap_default = getSandboxDefaultCap(getExecEnv()->last_frame->mb->sandbox_handle); \
    const pClass context = cap_unseal(pClass, Context, cap_context); \
	if (!context) { \
		printf("Warning: sandbox hasn't provided a valid context\n"); \
		return CHERI_FAIL; \
 	} */

#define LIBC_FUNCTION(NAME) \
	static register_t LIBC_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *cap_output, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4) { \
	__capability void **mem_output = arg_ptr(cap_output, sizeof(__capability void*), wc);

#define CALL_JNI(NAME) \
	JNI_##NAME (a1, a2, a3, a4, a5, a6, a7, cap_output, c1, c2, c3, c4)

#define CALL_LIBC(NAME) \
	LIBC_##NAME (a1, a2, a3, a4, a5, a6, a7, cap_output, c1, c2, c3, c4)

JNI_FUNCTION(GetVersion)
	return (*env)->GetVersion(env);
}

JNI_FUNCTION(FindClass)
	const char *className = arg_str(c1, 1, r);
	if (className == NULL)
		return CHERI_FAIL;

	jclass clazz = (*env)->FindClass(env, className);
	return_obj(clazz);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(ThrowNew)
	pClass clazz = arg_class(c1);
	if (clazz == NULL)
		return CHERI_FAIL;

	const char *msg = arg_str(c2, 1, r);
	return (*env)->ThrowNew(env, clazz, msg);
}

JNI_FUNCTION(ExceptionOccurred)
	jthrowable ex = (*env)->ExceptionOccurred(env);
	return_obj(ex);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(ExceptionDescribe)
	(*env)->ExceptionDescribe(env);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(ExceptionClear)
	(*env)->ExceptionClear(env);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(IsInstanceOf)
	pObject obj = arg_obj(c1);
	pClass clazz = arg_class(c2);
	if (clazz == NULL)
		return CHERI_FAIL;

	jboolean result = (*env)->IsInstanceOf(env, obj, clazz);
	return (register_t) result;
}

JNI_FUNCTION(GetMethodID)
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CHERI_FAIL;

	pMethodBlock result = (*env)->GetMethodID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkMethodAccess(result, context)) {
		jam_printf("Warning: sandbox requested a method outside its execution context\n");
		result = NULL;
	}
#endif
	return_mid(result);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(GetFieldID)
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CHERI_FAIL;

	pFieldBlock result = (*env)->GetFieldID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkFieldAccess(result, context)) {
		jam_printf("Warning: sandbox requested a field outside its execution context\n");
		result = NULL;
	}
#endif
	return_fid(result);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(GetStaticMethodID)
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CHERI_FAIL;

	pMethodBlock result = (*env)->GetStaticMethodID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkMethodAccess(result, context)) {
		jam_printf("Warning: sandbox requested a static method outside its execution context\n");
		result = NULL;
	}
#endif
	return_mid(result);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(NewStringUTF)
	const char *bytes = arg_str(c1, 1, r);
	if (bytes == NULL)
		return CHERI_FAIL;

	jstring str = (*env)->NewStringUTF(env, bytes);
	if (str == NULL)
		return CHERI_FAIL;

	return_obj(str);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(GetStringUTFLength)
	pObject string = arg_obj(c1);
	if (string == NULL)
		return CHERI_FAIL;

	if (!checkIsValidString(string))
		return CHERI_FAIL;
	return (*env)->GetStringUTFLength(env, string);
}

JNI_FUNCTION(GetStringUTFChars)
	pObject string = arg_obj(c1);
	if (!checkIsValidString(string))
		return CHERI_FAIL;
	jsize str_length = (*env)->GetStringUTFLength(env, string);

	__capability char *sandbox_buffer = arg_cap(c2, str_length + 1, w);
	if (sandbox_buffer == CNULL)
		return CHERI_FAIL;

	const char *host_buffer = (*env)->GetStringUTFChars(env, string, NULL);
	copyToSandbox(host_buffer, sandbox_buffer, str_length + 1);
	(*env)->ReleaseStringUTFChars(env, string, host_buffer);

	return CHERI_SUCCESS;
}

JNI_FUNCTION(ReleaseStringUTFChars)
	return CHERI_FAIL;
}

LIBC_FUNCTION(GetStdin)
	return_file(stdin);
	return fileno(stdin);
}

LIBC_FUNCTION(GetStdout)
	return_file(stdout);
	return fileno(stdout);
}

LIBC_FUNCTION(GetStderr)
	return_file(stderr);
	return fileno(stderr);
}

register_t cherijni_trampoline(register_t methodnum, register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, struct cheri_object system_object, __capability void *cap_output, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4) __attribute__((cheri_ccall)) {
	switch(methodnum) {
	case CHERIJNI_JNIEnv_GetVersion:
		return CALL_JNI(GetVersion);
	case CHERIJNI_JNIEnv_DefineClass:
		break;
	case CHERIJNI_JNIEnv_FindClass:
		return CALL_JNI(FindClass);
	case CHERIJNI_JNIEnv_FromReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_FromReflectedField:
		break;
	case CHERIJNI_JNIEnv_ToReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_GetSuperclass:
		break;
	case CHERIJNI_JNIEnv_IsAssignableFrom:
		break;
	case CHERIJNI_JNIEnv_ToReflectedField:
		break;
	case CHERIJNI_JNIEnv_Throw:
		break;
	case CHERIJNI_JNIEnv_ThrowNew:
		return CALL_JNI(ThrowNew);
	case CHERIJNI_JNIEnv_ExceptionOccurred:
		return CALL_JNI(ExceptionOccurred);
	case CHERIJNI_JNIEnv_ExceptionDescribe:
		return CALL_JNI(ExceptionDescribe);
	case CHERIJNI_JNIEnv_ExceptionClear:
		return CALL_JNI(ExceptionClear);
	case CHERIJNI_JNIEnv_FatalError:
		break;
	case CHERIJNI_JNIEnv_PushLocalFrame:
		break;
	case CHERIJNI_JNIEnv_PopLocalFrame:
		break;
	case CHERIJNI_JNIEnv_NewGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteLocalRef:
		break;
	case CHERIJNI_JNIEnv_IsSameObject:
		break;
	case CHERIJNI_JNIEnv_NewLocalRef:
		break;
	case CHERIJNI_JNIEnv_EnsureLocalCapacity:
		break;
	case CHERIJNI_JNIEnv_AllocObject:
		break;
	case CHERIJNI_JNIEnv_NewObject:
		break;
	case CHERIJNI_JNIEnv_NewObjectV:
		break;
	case CHERIJNI_JNIEnv_NewObjectA:
		break;
	case CHERIJNI_JNIEnv_GetObjectClass:
		break;
	case CHERIJNI_JNIEnv_IsInstanceOf:
		return CALL_JNI(IsInstanceOf);
	case CHERIJNI_JNIEnv_GetMethodID:
		return CALL_JNI(GetMethodID);
	case CHERIJNI_JNIEnv_CallObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_GetFieldID:
		return CALL_JNI(GetFieldID);
	case CHERIJNI_JNIEnv_GetObjectField:
		break;
	case CHERIJNI_JNIEnv_GetBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetByteField:
		break;
	case CHERIJNI_JNIEnv_GetCharField:
		break;
	case CHERIJNI_JNIEnv_GetShortField:
		break;
	case CHERIJNI_JNIEnv_GetIntField:
		break;
	case CHERIJNI_JNIEnv_GetLongField:
		break;
	case CHERIJNI_JNIEnv_GetFloatField:
		break;
	case CHERIJNI_JNIEnv_GetDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetObjectField:
		break;
	case CHERIJNI_JNIEnv_SetBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetByteField:
		break;
	case CHERIJNI_JNIEnv_SetCharField:
		break;
	case CHERIJNI_JNIEnv_SetShortField:
		break;
	case CHERIJNI_JNIEnv_SetIntField:
		break;
	case CHERIJNI_JNIEnv_SetLongField:
		break;
	case CHERIJNI_JNIEnv_SetFloatField:
		break;
	case CHERIJNI_JNIEnv_SetDoubleField:
		break;
	case CHERIJNI_JNIEnv_GetStaticMethodID:
		return CALL_JNI(GetStaticMethodID);
	case CHERIJNI_JNIEnv_CallStaticObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticObjectMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticObjectMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethodA:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethodV:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethodA:
		break;
	case CHERIJNI_JNIEnv_GetStaticFieldID:
		break;
	case CHERIJNI_JNIEnv_GetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_GetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_GetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_GetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_GetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_GetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_GetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_GetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_SetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_SetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_SetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_SetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_SetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_SetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_SetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_NewString:
		break;
	case CHERIJNI_JNIEnv_GetStringLength:
		break;
	case CHERIJNI_JNIEnv_GetStringChars:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringChars:
		break;
	case CHERIJNI_JNIEnv_NewStringUTF:
		return CALL_JNI(NewStringUTF);
	case CHERIJNI_JNIEnv_GetStringUTFLength:
		return CALL_JNI(GetStringUTFLength);
	case CHERIJNI_JNIEnv_GetStringUTFChars:
		return CALL_JNI(GetStringUTFChars);
	case CHERIJNI_JNIEnv_ReleaseStringUTFChars:
		return CALL_JNI(ReleaseStringUTFChars);
	case CHERIJNI_JNIEnv_GetArrayLength:
		break;
	case CHERIJNI_JNIEnv_NewObjectArray:
		break;
	case CHERIJNI_JNIEnv_GetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_SetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_NewBooleanArray:
		break;
	case CHERIJNI_JNIEnv_NewByteArray:
		break;
	case CHERIJNI_JNIEnv_NewCharArray:
		break;
	case CHERIJNI_JNIEnv_NewShortArray:
		break;
	case CHERIJNI_JNIEnv_NewIntArray:
		break;
	case CHERIJNI_JNIEnv_NewLongArray:
		break;
	case CHERIJNI_JNIEnv_NewFloatArray:
		break;
	case CHERIJNI_JNIEnv_NewDoubleArray:
		break;
	case CHERIJNI_JNIEnv_GetBooleanArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetByteArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetCharArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetShortArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetIntArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetLongArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetFloatArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetDoubleArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseBooleanArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseByteArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseCharArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseShortArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseIntArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseLongArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseFloatArrayElements:
		break;
	case CHERIJNI_JNIEnv_ReleaseDoubleArrayElements:
		break;
	case CHERIJNI_JNIEnv_GetBooleanArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetByteArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetCharArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetShortArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetIntArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetLongArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetFloatArrayRegion:
		break;
	case CHERIJNI_JNIEnv_GetDoubleArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetBooleanArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetByteArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetCharArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetShortArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetIntArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetLongArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetFloatArrayRegion:
		break;
	case CHERIJNI_JNIEnv_SetDoubleArrayRegion:
		break;
	case CHERIJNI_JNIEnv_RegisterNatives:
		break;
	case CHERIJNI_JNIEnv_UnregisterNatives:
		break;
	case CHERIJNI_JNIEnv_MonitorEnter:
		break;
	case CHERIJNI_JNIEnv_MonitorExit:
		break;
	case CHERIJNI_JNIEnv_GetJavaVM:
		break;
	case CHERIJNI_JNIEnv_GetStringRegion:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFRegion:
		break;
	case CHERIJNI_JNIEnv_GetPrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_ReleasePrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_GetStringCritical:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringCritical:
		break;
	case CHERIJNI_JNIEnv_NewWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_ExceptionCheck:
		break;
	case CHERIJNI_JNIEnv_NewDirectByteBuffer:
		break;
	case CHERIJNI_JNIEnv_GetDirectBufferAddress:
		break;
	case CHERIJNI_JNIEnv_GetDirectBufferCapacity:
		break;
	case CHERIJNI_JNIEnv_GetObjectRefType:
		break;

	case CHERIJNI_LIBC_GetStdin:
		return CALL_LIBC(GetStdin);
	case CHERIJNI_LIBC_GetStdout:
		return CALL_LIBC(GetStdout);
	case CHERIJNI_LIBC_GetStderr:
		return CALL_LIBC(GetStderr);

	default:
		break;
	}

	return CHERI_FAIL;
}

void initialiseCheriJNI() {
	cheri_system_user_register_fn(&cherijni_trampoline);
	INIT_SEAL(JavaObject);
	INIT_SEAL(Context);
	INIT_SEAL(MethodID);
	INIT_SEAL(FieldID);
	INIT_SEAL(FILE);

    class_String = findSystemClass0(SYMBOL(java_lang_String));
    registerStaticClassRef(&class_String);
}

#endif
