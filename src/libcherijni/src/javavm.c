#include "guest.h"

static jint cherijni_JavaVM_DestroyJavaVM(JavaVM *vm) {
	return JNI_ERR;
}

static jint cherijni_JavaVM_AttachCurrentThread(JavaVM *vm, void **penv, void *args) {
	return JNI_ERR;
}

static jint cherijni_JavaVM_DetachCurrentThread(JavaVM *vm) {
	return JNI_ERR;
}

static jint cherijni_JavaVM_GetEnv(JavaVM *vm, void **penv, jint version) {
	(*penv) = NULL;
	return JNI_EVERSION;
}

static jint cherijni_JavaVM_AttachCurrentThreadAsDaemon(JavaVM *vm, void **penv, void *args) {
	(*penv) = NULL;
	return JNI_ERR;
}

static struct _JNIInvokeInterface cherijni_JavaVM_data = {
	    NULL, NULL, NULL,
	    &cherijni_JavaVM_DestroyJavaVM,
	    &cherijni_JavaVM_AttachCurrentThread,
	    &cherijni_JavaVM_DetachCurrentThread,
	    &cherijni_JavaVM_GetEnv,
	    &cherijni_JavaVM_AttachCurrentThreadAsDaemon
	};

static JavaVM cherijni_JavaVM = &cherijni_JavaVM_data;

JavaVM *cherijni_getJavaVM() {
	return &cherijni_JavaVM;
}
