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

#include "sandbox.h"
#include "sandbox_shared.h"

extern void cherijni_initTrampoline(cherijniSandbox *sandbox);

char *cherijni_libName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof(".cheri") + 1);

   sprintf(buff, "%s.cheri", name);
   return buff;
}

#define cNULL	(cheri_zerocap())
#define CInvoke_7_6(handle, op, a1, a2, a3, a4, a5, a6, a7, c5, c6, c7, c8, c9, c10)                         \
		sandbox_object_cinvoke(                                                                              \
				((cherijniSandbox *) handle)->objectp, op,                                           \
	            (register_t) a1, (register_t) a2, (register_t) a3, (register_t) a4,                          \
                (register_t) a5, (register_t) a6, (register_t) a7,                                           \
	            sandbox_object_getsystemobject(((cherijniSandbox *) handle)->objectp).co_codecap,    \
	            sandbox_object_getsystemobject(((cherijniSandbox *) handle)->objectp).co_datacap,    \
	            c5, c6, c7, c8, c9, c10)
#define CInvoke_1_1(handle, op, a1, c5)  CInvoke_7_6(handle, op, a1, 0, 0, 0, 0, 0, 0, c5, cNULL, cNULL, cNULL, cNULL, cNULL)
#define CInvoke_1_0(handle, op, a1)      CInvoke_1_1(handle, op, a1, cNULL)
#define CInvoke_0_1(handle, op, c5)      CInvoke_1_1(handle, op, 0, c5)
#define CInvoke_0_0(handle, op)          CInvoke_1_1(handle, op, 0, cNULL)
#define CString(str)                     (cheri_ptrperm(str, strlen(str) + 1, CHERI_PERM_LOAD))
#define CObject(ptr)                     (cheri_ptr((void*) ptr, sizeof(Object)))

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

	/* Register trampoline */
	if (sandbox_class_method_declare(sandbox->classp, CHERIJNI_TRAMPOLINE_JNIENV, "trampoline_JNIEnv") < 0) {
		sysFree(sandbox);
		return NULL;
	}

	return sandbox;
}

void cherijni_runTests(void *handle, JNIEnv *env) {
	CInvoke_0_0(handle, CHERIJNI_METHOD_TEST);
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

uintptr_t *cherijni_callMethod(void* handle, void *native_func, JNIEnv *env, pClass class, char *sig, uintptr_t *ostack) {
	__capability void *cEnv = cheri_ptr(env, sizeof(JNIEnv*));

	uintptr_t *_ostack = ostack;

	/* Is it an instance call? */

	__capability void *cThis;
//	if (class == NULL)
//		cThis = CObject(*(_ostack++));
//	else
//		cThis = CObject(class);

	/* Count the arguments */

	size_t cPrimitiveArgs = 0;
	size_t cObjectArgs = 0;

	forEachArgument(sig,
		/* single primitives */ { (cPrimitiveArgs++); },
		/* double primitives */ { (cPrimitiveArgs++); },
		/* objects           */ { (cObjectArgs++); });

	// TODO: check number of arguments

	register_t pPrimitiveArgs [6] = {0, 0, 0, 0, 0, 0};
	__capability void *pObjectArgs [5] = {cNULL, cNULL, cNULL, cNULL, cNULL};

	register_t *_pPrimitiveArgs = pPrimitiveArgs;
	__capability void **_pObjectArgs = pObjectArgs;

	// TODO: remove this and use the above
	if (class == NULL)
		*(_pPrimitiveArgs++) = *(_ostack++);
	else
		*(_pPrimitiveArgs++) = class;

	forEachArgument(sig,
		/* single primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); },
		/* double primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); _ostack++; },
		/* objects           */ { *(_pPrimitiveArgs++) = *(_ostack++); }); // *(_pObjectArgs++) = CObject(*(_ostack++));

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

	jam_printf("Calling cherijni function %p with handle %p and %d args\n", native_func, handle, cPrimitiveArgs + cObjectArgs);

	register_t result = CInvoke_7_6(handle, CHERIJNI_METHOD_RUN, native_func, a0, a1, a2, a3, a4, a5, cThis, c0, c1, c2, c3, c4);

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

#endif
