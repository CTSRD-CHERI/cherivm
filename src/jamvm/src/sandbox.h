#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#ifdef JNI_CHERI

#include "jni.h"

#define DEFAULT_SANDBOX_MEM 4*MB

extern char *cheriJNI_libName(char *name);
extern void *cheriJNI_lookup(void *handle, char *methodName);
extern void *cheriJNI_open(char *path);
extern jint cheriJNI_callOnLoadUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved);
extern uintptr_t *cheriJNI_callMethod(void* handle, void *native_func, JNIEnv *env, pClass class, char *sig, uintptr_t *ostack);

typedef struct cherijni_sandbox {
	struct sandbox_class    *classp;
	struct sandbox_object   *objectp;
} cherijniSandbox;

#endif

#endif //__SANDBOX_H__
