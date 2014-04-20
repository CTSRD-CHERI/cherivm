#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#ifdef JNI_CHERI

#include "jni.h"

extern void cherijni_init();
extern char *cherijni_libName(char *name);
extern void *cherijni_lookup(void *handle, char *methodName);
extern void *cherijni_open(char *path);
extern jint cherijni_callOnLoadUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved);
extern uintptr_t *cherijni_callMethod(void* handle, void *native_func, JNIEnv *env, pClass class, char *sig, uintptr_t *ostack);
extern void cherijni_runTests(void *handle, JNIEnv *env);

#endif

#endif //__SANDBOX_H__
