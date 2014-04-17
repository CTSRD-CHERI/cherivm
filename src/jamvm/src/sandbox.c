#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#include "jam.h"
#include "sandbox.h"

#ifdef JNI_CHERI

#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>

#include <cheri/sandbox.h>

#define CHERIJNI_METHOD_LOOKUP      0
#define CHERIJNI_METHOD_ONLOAD      1
#define CHERIJNI_METHOD_ONUNLOAD    2
#define CHERIJNI_METHOD_RUN         3

struct cherijni_sandbox {
	struct sandbox_class    *classp;
	struct sandbox_object   *objectp;
};

void *cheriJNI_open(char *path) {
	/*
	 *	Initialize the sandbox class and object
	 */

	struct cherijni_sandbox *sandbox = sysMalloc(sizeof(struct cherijni_sandbox));

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

// TODO: make sure only one thread ever enters the sandbox !!!
// TODO: check the return value? -1 *may* mean that a trap happened inside the sandbox (will be replaced with signals)

#define cNULL	(cheri_zerocap())
#define CInvoke_7_6(handle, op, a1, a2, a3, a4, a5, a6, a7, c5, c6, c7, c8, c9, c10)                         \
		sandbox_object_cinvoke(                                                                              \
				((struct cherijni_sandbox *) handle)->objectp, op,                                           \
	            (register_t) a1, (register_t) a2, (register_t) a3, (register_t) a4,                          \
                (register_t) a5, (register_t) a6, (register_t) a7,                                           \
	            sandbox_object_getsystemobject(((struct cherijni_sandbox *) handle)->objectp).co_codecap,    \
	            sandbox_object_getsystemobject(((struct cherijni_sandbox *) handle)->objectp).co_datacap,    \
	            c5, c6, c7, c8, c9, c10)
#define CInvoke_1_1(handle, op, a1, c5)  CInvoke_7_6(handle, op, a1, 0, 0, 0, 0, 0, 0, c5, cNULL, cNULL, cNULL, cNULL, cNULL)
#define CInvoke_1_0(handle, op, a1)      CInvoke_1_1(handle, op, a1, cNULL)
#define CInvoke_0_1(handle, op, c5)      CInvoke_1_1(handle, op, 0, c5)
#define CString(str)                     (cheri_ptrperm(str, strlen(str) + 1, CHERI_PERM_LOAD))
#define CObject(ptr)                     (cheri_ptr((void*) ptr, sizeof(Object)))

void *cheriJNI_lookup(void *handle, char *methodName) {
	return (void*) CInvoke_0_1(handle, CHERIJNI_METHOD_LOOKUP, CString(methodName));
}

static jint cheriJNI_callOnLoadUnload(void *handle, void *ptr, u_int op, JavaVM *jvm, void *reserved) {
	__capability void *cJvm = cheri_ptr(jvm, sizeof(JavaVM));
	return (jint) CInvoke_1_0(handle, op, ptr);
}

jint cheriJNI_callOnLoad(void *handle, void *ptr, JavaVM *jvm, void *reserved) {
	return cheriJNI_callOnLoadUnload(handle, ptr, CHERIJNI_METHOD_ONLOAD, jvm, reserved);
}

jint cheriJNI_callOnUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved) {
	return cheriJNI_callOnLoadUnload(handle, ptr, CHERIJNI_METHOD_ONUNLOAD, jvm, reserved);
}

#define SCAN_SIGNATURE(sig)               \
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

uintptr_t *cheriJNI_callMethod(void* handle, void *native_func, JNIEnv *env, pClass class, char *sig, uintptr_t *ostack) {
	__capability void *cEnv = cheri_ptr(env, sizeof(JNIEnv*));

	uintptr_t *_ostack = ostack;

	/* Is it an instance call? */

	__capability void *cThis;
	if (class == NULL)
		cThis = CObject(*(_ostack++));
	else
		cThis = CObject(class);

	/* Count the arguments */

	size_t cPrimitiveArgs = 0;
	size_t cObjectArgs = 0;

	#define SCAN_PRIM_SINGLE    { (cPrimitiveArgs++); }
	#define SCAN_PRIM_DOUBLE    { (cPrimitiveArgs++); }
	#define SCAN_OBJECT         { (cObjectArgs++); }
	SCAN_SIGNATURE(sig);
	#undef SCAN_PRIM_SINGLE
	#undef SCAN_PRIM_DOUBLE
	#undef SCAN_OBJECT

	// TODO: check number of arguments

	register_t pPrimitiveArgs [6] = {0, 0, 0, 0, 0, 0};
	__capability void *pObjectArgs [5] = {cNULL, cNULL, cNULL, cNULL, cNULL};

	register_t *_pPrimitiveArgs = pPrimitiveArgs;
	__capability void **_pObjectArgs = pObjectArgs;

	#define SCAN_PRIM_SINGLE    { *(_pPrimitiveArgs++) = *(_ostack++); }
	#define SCAN_PRIM_DOUBLE    { *(_pPrimitiveArgs++) = *(_ostack++); _ostack++; }
	#define SCAN_OBJECT         { *(_pObjectArgs++) = CObject(*(_ostack++)); }
	SCAN_SIGNATURE(sig);
	#undef SCAN_PRIM_SINGLE
	#undef SCAN_PRIM_DOUBLE
	#undef SCAN_OBJECT

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
	__capability void* c4 = pObjectArgs[4];

	register_t result = CInvoke_7_6(handle, CHERIJNI_METHOD_RUN, native_func, a0, a1, a2, a3, a4, a5, cThis, c0, c1, c2, c3, c4);

	// TODO: put return value on the stack

	return ostack;
}

#endif
