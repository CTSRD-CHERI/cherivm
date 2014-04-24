#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#include "jam.h"

#ifdef JNI_CHERI

#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>

#include <cheri/sandbox.h>
#include <cheri/cheri_class.h>
#include <cheri/cheri_enter.h>
#include <cheri/cheri_system.h>

#include "sandbox.h"
#include "sandbox_shared.h"

extern const JNIEnv globalJNIEnv;

typedef struct cherijni_sandbox {
        struct sandbox_class    *classp;
        struct sandbox_object   *objectp;
} cherijniSandbox;

static void              *cherijni_unseal (__capability void *datacap, __capability void *sealcap);

#define DEFINE_SEAL_VARS(TYPE, NAME)                                \
        static uintptr_t          type_##NAME;                      \
        static __capability void *sealcap_##NAME;                   \

#define cap_seal(NAME, data) ((data == NULL) ? (__capability void*) NULL : cheri_sealdata(cheri_ptrperm(data, sizeof(uintptr_t), CHERI_PERM_LOAD), sealcap_##NAME))
#define cap_unseal(TYPE, NAME, datacap) ((TYPE) cherijni_unseal(datacap, sealcap_##NAME))
#define INIT_SEAL(NAME) sealcap_##NAME = cheri_ptrtype(&type_##NAME, sizeof(uintptr_t), 0); // sets PERMIT_SEAL

DEFINE_SEAL_VARS(pObject, JavaObject)
DEFINE_SEAL_VARS(pClass, Context)
DEFINE_SEAL_VARS(pFieldBlock, FieldID)

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
		sandbox_object_cinvoke(                                                                              \
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
	return (void*) CInvoke_0_1(handle, CHERIJNI_METHOD_LOOKUP, CString(methodName));
}

jint cherijni_callOnLoadUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved) {
	__capability void *cJvm = cheri_ptr(jvm, sizeof(JavaVM));
	return (jint) CInvoke_1_0(handle, CHERIJNI_METHOD_ONLOAD_ONUNLOAD, ptr);
}

#define forEachArgument(sig, SCAN_PRIM_SINGLE, SCAN_PRIM_DOUBLE, SCAN_OBJECT) \
{                                         \
	char *s = sig;                        \
	char c = s[1];                        \
	while (c != ')') {                    \
		if (c == 'D' || c == 'J')         \
			SCAN_PRIM_DOUBLE              \
		else {                            \
			if (c == 'L' || c == '[')     \
				SCAN_OBJECT               \
			else                          \
				SCAN_PRIM_SINGLE          \
			                              \
			while (c == '[')              \
				c = (++s)[1];             \
			if (c == 'L')                 \
				while (c != ';')          \
					c = (++s)[1];         \
		}                                 \
		c = (++s)[1];                     \
	}                                     \
}

#define forReturnType(sig, SCAN_VOID, SCAN_PRIM_SINGLE, SCAN_PRIM_DOUBLE, SCAN_OBJECT) \
{                                         \
	char *s = sig;                        \
	char c = s[1];                        \
	while (c != ')')                      \
		c = (++s)[1];                     \
	c = (++s)[1];                         \
	if (c == 'V')                         \
        SCAN_VOID                         \
	else if (c == 'D' || c == 'J')        \
		SCAN_PRIM_DOUBLE                  \
	else if (c == 'L' || c == '[')        \
		SCAN_OBJECT                       \
	else                                  \
		SCAN_PRIM_SINGLE                  \
}

uintptr_t *cherijni_callMethod(void* handle, void *native_func, pClass class, char *sig, uintptr_t *ostack) {
	__capability void *cContext = cap_seal(Context, class);
	__capability void *cThis;
	uintptr_t *_ostack = ostack;

	/* Is it an instance call? */

	if (class == NULL)
		cThis = cap_seal(JavaObject, (pObject) *(_ostack++));
	else
		cThis = cap_seal(JavaObject, class);

	/* Count the arguments */

	size_t cPrimitiveArgs = 0;
	size_t cObjectArgs = 0;

	forEachArgument(sig,
		/* single primitives */ { (cPrimitiveArgs++); },
		/* double primitives */ { (cPrimitiveArgs++); },
		/* objects           */ { (cObjectArgs++); });

	// TODO: check number of arguments

	register_t pPrimitiveArgs [6] = {0, 0, 0, 0, 0, 0};
	__capability void *pObjectArgs [4] = {CNULL, CNULL, CNULL, CNULL};

	register_t *_pPrimitiveArgs = pPrimitiveArgs;
	__capability void **_pObjectArgs = pObjectArgs;

	forEachArgument(sig,
		/* single primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); },
		/* double primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); _ostack++; },
		/* objects           */ { *(_pObjectArgs++) = cap_seal(JavaObject, (pObject) *(_ostack++)); });

	register_t a0 = pPrimitiveArgs[0];
	register_t a1 = pPrimitiveArgs[1];
	register_t a2 = pPrimitiveArgs[2];
	register_t a3 = pPrimitiveArgs[3];
	register_t a4 = pPrimitiveArgs[4];
	register_t a5 = pPrimitiveArgs[5];

	__capability void* c0 = pObjectArgs[0];
	__capability void* c1 = pObjectArgs[1];
	__capability void* c2 = pObjectArgs[2];
	__capability void* c3 = pObjectArgs[3];

	jam_printf("Calling cherijni function %p with handle %p and %d args\n", native_func, handle, cPrimitiveArgs + cObjectArgs);

	register_t result = CInvoke_7_6(handle, CHERIJNI_METHOD_RUN, native_func, a0, a1, a2, a3, a4, a5, cContext, cThis, c0, c1, c2, c3);

	// TODO: if it returns (-1), it *might* have failed executing!
	// TODO: ask rwatson: how much would it take to return the error code in $v1?

	jam_printf("Sandbox returned %p\n", (void*) result);

	/* Put the return value back on stack */

	forReturnType(sig,
		/* void             */ { },
		/* single primitive */ { *(ostack++) = result; },
		/* double primitive */ { *(ostack++) = result; ostack++; },
		/* objects          */ { *(ostack++) = result; });

	return ostack;
}

#define arg_obj(ptr)    cap_unseal(pObject, JavaObject, getCapabilityAt(convertSandboxPointer(ptr, cap_default)))
#define arg_class(ptr)  checkIsClass(arg_obj(ptr))
#define arg_str(ptr)    ((const char*) convertSandboxPointer(ptr, cap_default))
#define return_obj(obj)    { *mem_output = cap_seal(JavaObject, obj); }
#define return_fid(field)  { (*mem_output) = cap_seal(FieldID, field); }

static inline void *convertSandboxPointer(register_t guest_ptr, __capability void *cap_default) {
	// NULL pointer will be zero
	if (guest_ptr == 0)
		return NULL;

	// check it is within bounds
	if (guest_ptr < 0 || guest_ptr >= cheri_getlen(cap_default)) {
		jam_printf("Warning: sandbox gave a pointer outside of its bounds\n");
		return NULL;
	}

	// translate
	return (void*) cheri_incbase(cap_default, guest_ptr);
}

static inline __capability void *getCapabilityAt(void *ptr) {
	if (((uintptr_t) ptr) & (sizeof(__capability void*) - 1)) {
		jam_printf("Warning: sandbox gave a misaligned capability pointer\n");
		return cheri_zerocap();
	}

	return *((__capability void**) ptr);
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

#define JNI_FUNCTION(NAME) \
	static register_t NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *cap_default, __capability void *cap_context, __capability void *cap_output, __capability void *c1, __capability void *c2) { \
	JNIEnv *env = &globalJNIEnv; \
	__capability void **mem_output = (void*) cap_output; \
	const pClass context = cap_unseal(pClass, Context, cap_context); \
	if (!context) \
		return CHERI_FAIL;

#define CALL_JNI(NAME) \
	NAME (a1, a2, a3, a4, a5, a6, a7, cap_default, cap_context, cap_output, c1, c2)

JNI_FUNCTION(GetVersion)
	return (*env)->GetVersion(env);
}

JNI_FUNCTION(FindClass)
	const char *className = arg_str(a1);
	jclass clazz = (*env)->FindClass(env, className);
	return_obj(clazz);
	return CHERI_SUCCESS;
}

JNI_FUNCTION(IsInstanceOf)
	pObject obj = arg_obj(a1);
	pClass clazz = arg_class(a2);
	jboolean result = (*env)->IsInstanceOf(env, obj, clazz);
	return (register_t) result;
}

JNI_FUNCTION(GetFieldID)
	pClass clazz = arg_class(a1);
	const char *name = arg_str(a2);
	const char *sig = arg_str(a3);
	pFieldBlock result = (*env)->GetFieldID(env, clazz, name, sig);
	if (!checkFieldAccess(result, context))
		result = NULL;
	return_fid(result);
	return CHERI_SUCCESS;
}

register_t cherijni_trampoline(register_t methodnum, register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, struct cheri_object system_object, __capability void *cap_default, __capability void *cap_context, __capability void *cap_output, __capability void *c1, __capability void *c2) __attribute__((cheri_ccall)) {
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
		break;
	case CHERIJNI_JNIEnv_ExceptionOccurred:
		break;
	case CHERIJNI_JNIEnv_ExceptionDescribe:
		break;
	case CHERIJNI_JNIEnv_ExceptionClear:
		break;
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
		break;
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
		break;
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
		break;
	case CHERIJNI_JNIEnv_GetStringUTFLength:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFChars:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringUTFChars:
		break;
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
	}

	return CHERI_FAIL;
}

void cherijni_init() {
	cheri_system_user_register_fn(&cherijni_trampoline);
	INIT_SEAL(JavaObject);
	INIT_SEAL(Context);
	INIT_SEAL(FieldID);
}

#endif
