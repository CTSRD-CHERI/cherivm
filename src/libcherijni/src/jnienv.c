#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

static jclass FindClass(JNIEnv *env, const char *className) {
	printf("[JNIEnv %s stub]\n", __func__);
	return NULL;
}

static jthrowable ExceptionOccured(JNIEnv *env) {
	printf("[JNIEnv %s stub]\n", __func__);
	return NULL;
}

static void ExceptionClear(JNIEnv *env) {
	printf("[JNIEnv %s stub]\n", __func__);
}

static jint ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
	printf("[JNIEnv %s stub]\n", __func__);
	return (-1);
}

JNIEnv *cherijni_getJNIEnv() {
	size_t total_size = sizeof(JNIEnv*) + sizeof(struct _JNINativeInterface);
	JNIEnv *ppEnv = (JNIEnv*) malloc(total_size);
	memset(ppEnv, 0, total_size);
	struct _JNINativeInterface * pEnv = (struct _JNINativeInterface *) (ppEnv + 1);

	pEnv->ExceptionClear = &ExceptionClear;
	pEnv->ExceptionOccurred = &ExceptionOccured;
	pEnv->FindClass = &FindClass;
	pEnv->ThrowNew = &ThrowNew;

	*(ppEnv) = pEnv;
	return ppEnv;
}

void cherijni_destroyJNIEnv(JNIEnv *ppEnv) {
	free(ppEnv);
}
