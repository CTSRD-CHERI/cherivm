#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

void Java_SodiumTest_initIDs(JNIEnv *env, jclass clazz);
jint Java_SodiumTest_getPublicKeySize(JNIEnv *env, jclass clazz);
jint Java_SodiumTest_getPrivateKeySize(JNIEnv *env, jclass clazz);
jint Java_SodiumTest_getNonceSize(JNIEnv *env, jclass clazz);
void Java_SodiumTest_generateKeyPair(JNIEnv *env, jclass clazz, jobject bufPublic, jobject bufPrivate);
void Java_SodiumTest_generateNonce(JNIEnv *env, jclass clazz, jobject bbufNonce);
jint Java_SodiumTest_encryptLength(JNIEnv *env, jclass clazz, jint length);
void Java_SodiumTest_encryptData(JNIEnv *env, jclass clazz, jobject bbufData, jobject bbufSenderPrivate, jobject bbufRecipientPublic, jobject bbufNonce, jobject bbufOutput);
jint Java_SodiumTest_decryptLength(JNIEnv *env, jclass clazz, jint length);
void Java_SodiumTest_decryptData(JNIEnv *env, jclass clazz, jobject bbufData, jobject bbufRecipientPrivate, jobject bbufSenderPublic, jobject bbufNonce, jobject bbufOutput);

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{ "Java_SodiumTest_initIDs", Java_SodiumTest_initIDs, 1 },
	{ "Java_SodiumTest_getPublicKeySize", Java_SodiumTest_getPublicKeySize, 2 },
	{ "Java_SodiumTest_getPrivateKeySize", Java_SodiumTest_getPrivateKeySize, 2 },
	{ "Java_SodiumTest_getNonceSize", Java_SodiumTest_getNonceSize, 2 },
	{ "Java_SodiumTest_generateKeyPair", Java_SodiumTest_generateKeyPair, 1 },
	{ "Java_SodiumTest_generateNonce", Java_SodiumTest_generateNonce, 1 },
	{ "Java_SodiumTest_encryptLength", Java_SodiumTest_encryptLength, 2 },
	{ "Java_SodiumTest_encryptData", Java_SodiumTest_encryptData, 1 },
	{ "Java_SodiumTest_decryptLength", Java_SodiumTest_decryptLength, 2 },
	{ "Java_SodiumTest_decryptData", Java_SodiumTest_decryptData, 1 },
	{NULL, NULL, 0}
};
