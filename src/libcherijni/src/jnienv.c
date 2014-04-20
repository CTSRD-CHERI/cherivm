#include "guest.h"

static jint GetVersion(JNIEnv *env) {
	return (jint) (cheri_invoke(cherijni_SystemObject,
	    CHERIJNI_JNIEnv_GetVersion, 0, 0, 0, 0, 0, 0, 0,
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(),
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(),
	    cheri_zerocap(), cheri_zerocap()));
}

static jclass FindClass(JNIEnv *env, const char *className) {
	return (jclass) (cheri_invoke(cherijni_SystemObject,
	    CHERIJNI_JNIEnv_FindClass, className, 0, 0, 0, 0, 0, 0,
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(),
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(),
	    cheri_zerocap(), cheri_zerocap()));
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

	pEnv->GetVersion = &GetVersion;
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
