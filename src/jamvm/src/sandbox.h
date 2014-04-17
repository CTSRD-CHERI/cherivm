#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#ifdef JNI_CHERI

#include "jni.h"

extern char *cheriJNI_libName(char *name);
extern void *cheriJNI_lookup(void *handle, char *methodName);
extern void *cheriJNI_open(char *path);
extern jint cheriJNI_callOnLoad(void *handle, void *ptr, JavaVM *jvm, void *reserved);
extern jint cheriJNI_callOnUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved);
extern uintptr_t *cheriJNI_callMethod(void* handle, void *native_func, JNIEnv *env, pClass class, char *sig, uintptr_t *ostack);

#endif

#endif //__SANDBOX_H__
