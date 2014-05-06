#include "guest.h"

static jint DestroyJavaVM(JavaVM *vm) {
	return JNI_ERR;
}

static jint AttachCurrentThread(JavaVM *vm, void **penv, void *args) {
	return JNI_ERR;
}

static jint DetachCurrentThread(JavaVM *vm) {
	return JNI_ERR;
}

static jint GetEnv(JavaVM *vm, void **penv, jint version) {
	(*penv) = cherijni_getJNIEnv();
	return JNI_OK;
}

static jint AttachCurrentThreadAsDaemon(JavaVM *vm, void **penv, void *args) {
	(*penv) = NULL;
	return JNI_ERR;
}

static struct _JNIInvokeInterface cherijni_JavaVM_data = {
	    NULL, NULL, NULL,
	    DestroyJavaVM,
	    AttachCurrentThread,
	    DetachCurrentThread,
	    GetEnv,
	    AttachCurrentThreadAsDaemon
	};

static JavaVM cherijni_JavaVM = &cherijni_JavaVM_data;

JavaVM *cherijni_getJavaVM() {
	return &cherijni_JavaVM;
}
