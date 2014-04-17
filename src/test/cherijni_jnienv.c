#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

JNIEnv *cherijni_getJNIEnv() {
	JNIEnv pEnv = (JNIEnv) malloc(sizeof(struct _JNINativeInterface));
	memset(pEnv, 0, sizeof(struct _JNINativeInterface));

	JNIEnv *ppEnv = (JNIEnv*) malloc(sizeof(JNIEnv*));
	*(ppEnv) = pEnv;
	return ppEnv;
}
