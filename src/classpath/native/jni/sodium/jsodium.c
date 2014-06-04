#include <sys/stat.h>
#include <fcntl.h>
#include "jni.h"
#include <sodium.h>
#include <unistd.h>

enum JCL_buffer_type { DIRECT, HEAP, ARRAY, UNKNOWN };

struct JCL_buffer
{
  enum JCL_buffer_type type;
  jbyte *ptr;
  jint offset;
  jint position;
  jint limit;
  jint count;
};

static jmethodID get_position_mid;
static jmethodID set_position_mid;
static jmethodID get_limit_mid;
static jmethodID has_array_mid;
static jmethodID array_mid;
static jmethodID array_offset_mid;

JNIEXPORT void JNICALL JCL_ThrowException(JNIEnv * env, const char *className, const char *errMsg) {
	jclass excClass;

	if ((*env)->ExceptionOccurred (env))
		(*env)->ExceptionClear (env);

	excClass = (*env)->FindClass (env, className);
	if (excClass == NULL) {
		jclass errExcClass;
		errExcClass = (*env)->FindClass (env, "java/lang/ClassNotFoundException");
		if (errExcClass == NULL) {
			errExcClass = (*env)->FindClass (env, "java/lang/InternalError");
			if (errExcClass == NULL) {
				fprintf(stderr, "JCL: Utterly failed to throw exeption %s with message %s", className, errMsg);
				return;
			}
		}

		(*env)->ThrowNew (env, errExcClass, className);
	}
	(*env)->ThrowNew (env, excClass, errMsg);
}

jmethodID get_method_id(JNIEnv *env,  jclass clazz, const char *name, const char *sig) {
	jmethodID mid = (*env)->GetMethodID(env, clazz, name, sig);
	if (mid == NULL)
		JCL_ThrowException(env, "java/lang/InternalError", name);
	return mid;
}

int JCL_init_buffer(JNIEnv *env, struct JCL_buffer *buf, jobject bbuf) {
	buf->position = (*env)->CallIntMethod(env, bbuf, get_position_mid);
	buf->limit = (*env)->CallIntMethod(env, bbuf, get_limit_mid);
	buf->offset = 0;
	buf->count = 0;
	buf->type = UNKNOWN;

	jboolean has_array;
	has_array = (*env)->CallBooleanMethod(env, bbuf, has_array_mid);

	if (has_array == JNI_TRUE) {
		jbyteArray arr;
		buf->offset = (*env)->CallIntMethod(env, bbuf, array_offset_mid);
		arr = (*env)->CallObjectMethod(env, bbuf, array_mid);
		buf->ptr = (*env)->GetByteArrayElements(env, arr, 0);
		buf->type = ARRAY;
		(*env)->DeleteLocalRef(env, arr);
	} else {
		jclass clazz = (*env)->FindClass(env, "java/lang/InternalError");
		(*env)->ThrowNew(env, clazz, "Unsupported buffer type");
	}

	return 0;
}

void JCL_release_buffer(JNIEnv *env, struct JCL_buffer *buf, jobject bbuf, jint action) {
	jbyteArray arr;

	if (buf->count > 0) {
		jobject bbufTemp;
		bbufTemp = (*env)->CallObjectMethod(env, bbuf, set_position_mid, buf->position + buf->count);
		(*env)->DeleteLocalRef(env, bbufTemp);
	}

	switch (buf->type)
	{
	case DIRECT:
	case HEAP:
		break;
	case ARRAY:
		arr = (*env)->CallObjectMethod(env, bbuf, array_mid);
		(*env)->ReleaseByteArrayElements(env, arr, buf->ptr, action);
		(*env)->DeleteLocalRef(env, arr);
		break;
	case UNKNOWN:
		break;
	}
}

void printBuffer(jbyte *buf, int len) {
	int i;
	for (i = 0; i < len; i++)
		printf("%02X", (unsigned char) buf[i]);
}

JNIEXPORT void JNICALL Java_SodiumTest_initIDs(JNIEnv *env, jclass clazz) {
	jclass bufferClass = (*env)->FindClass(env, "java/nio/Buffer");
	jclass byteBufferClass = (*env)->FindClass(env, "java/nio/ByteBuffer");

	get_position_mid = get_method_id(env, bufferClass, "position", "()I");
	set_position_mid = get_method_id(env, bufferClass, "position", "(I)Ljava/nio/Buffer;");
	get_limit_mid = get_method_id(env, bufferClass, "limit", "()I");

	has_array_mid = get_method_id(env, byteBufferClass, "hasArray", "()Z");
	array_mid = get_method_id(env, byteBufferClass, "array", "()[B");
	array_offset_mid = get_method_id(env, byteBufferClass, "arrayOffset", "()I");
}

JNIEXPORT jint JNICALL Java_SodiumTest_getPublicKeySize(JNIEnv *env, jclass clazz) {
	return crypto_box_PUBLICKEYBYTES;
}

JNIEXPORT jint JNICALL Java_SodiumTest_getPrivateKeySize(JNIEnv *env, jclass clazz) {
	return crypto_box_SECRETKEYBYTES;
}

JNIEXPORT jint JNICALL Java_SodiumTest_getNonceSize(JNIEnv *env, jclass clazz) {
	return crypto_box_NONCEBYTES;
}

JNIEXPORT void JNICALL Java_SodiumTest_generateKeyPair(JNIEnv *env, jclass clazz, jobject bbufPublic, jobject bbufPrivate) {
	struct JCL_buffer bufPublic, bufPrivate;
	JCL_init_buffer(env, &bufPublic, bbufPublic);
	JCL_init_buffer(env, &bufPrivate, bbufPrivate);

	crypto_box_keypair(bufPublic.ptr, bufPrivate.ptr);

	JCL_release_buffer(env, &bufPublic, bbufPublic, 0);
	JCL_release_buffer(env, &bufPrivate, bbufPrivate, 0);
}

JNIEXPORT void JNICALL Java_SodiumTest_generateNonce(JNIEnv *env, jclass clazz, jobject bbufNonce) {
	struct JCL_buffer bufNonce;
	JCL_init_buffer(env, &bufNonce, bbufNonce);
	randombytes(bufNonce.ptr, crypto_box_NONCEBYTES);
	JCL_release_buffer(env, &bufNonce, bbufNonce, 0);
}

#define MAX_MSG_SIZE 1400

int is_zero(const jbyte *data, int len) {
	int i;
	int rc;

	rc = 0;
	for(i = 0; i < len; ++i) {
		rc |= data[i];
	}

	return rc;
}

JNIEXPORT jint JNICALL Java_SodiumTest_encryptLength(JNIEnv *env, jclass clazz, jint length) {
	return crypto_box_ZEROBYTES + length;
}

JNIEXPORT void JNICALL Java_SodiumTest_encryptData(JNIEnv *env, jclass clazz, jobject bbufData, jobject bbufSenderPrivate, jobject bbufRecipientPublic, jobject bbufNonce, jobject bbufOutput) {
	struct JCL_buffer bufData, bufNonce, bufSenderPrivate, bufRecipientPublic, bufOutput;
	JCL_init_buffer(env, &bufData, bbufData);
	JCL_init_buffer(env, &bufNonce, bbufNonce);
	JCL_init_buffer(env, &bufSenderPrivate, bbufSenderPrivate);
	JCL_init_buffer(env, &bufRecipientPublic, bbufRecipientPublic);
	JCL_init_buffer(env, &bufOutput, bbufOutput);

	jbyte temp_plain[MAX_MSG_SIZE];
	jbyte temp_encrypted[MAX_MSG_SIZE];
	int rc;
	int length = bufData.limit;

	if (length + crypto_box_ZEROBYTES >= MAX_MSG_SIZE) {
		JCL_ThrowException(env, "java/lang/Exception", "Data too long");
		return;
	}

	memset(temp_plain, '\0', crypto_box_ZEROBYTES);
	memcpy(temp_plain + crypto_box_ZEROBYTES, bufData.ptr, length);

	rc = crypto_box(temp_encrypted, temp_plain, crypto_box_ZEROBYTES + length, bufNonce.ptr, bufRecipientPublic.ptr, bufSenderPrivate.ptr);
	if (rc != 0) {
		JCL_ThrowException(env, "java/lang/Exception", "Encryption failed (return code)");
		return;
	}

	if (is_zero(temp_plain, crypto_box_BOXZEROBYTES) != 0) {
		JCL_ThrowException(env, "java/lang/Exception", "Encryption failed (zero bytes)");
		return;
	}

	if (crypto_box_ZEROBYTES + length > bufOutput.limit) {
		JCL_ThrowException(env, "java/lang/Exception", "Output buffer too short");
		return;
	}

	memcpy(bufOutput.ptr, temp_encrypted + crypto_box_BOXZEROBYTES, crypto_box_ZEROBYTES + length);

	JCL_release_buffer(env, &bufData, bbufData, JNI_ABORT);
	JCL_release_buffer(env, &bufNonce, bbufNonce, JNI_ABORT);
	JCL_release_buffer(env, &bufSenderPrivate, bbufSenderPrivate, JNI_ABORT);
	JCL_release_buffer(env, &bufRecipientPublic, bbufRecipientPublic, JNI_ABORT);
	JCL_release_buffer(env, &bufOutput, bbufOutput, 0);
}

JNIEXPORT jint JNICALL Java_SodiumTest_decryptLength(JNIEnv *env, jclass clazz, jint length) {
	return crypto_box_BOXZEROBYTES + length - crypto_box_ZEROBYTES;
}

JNIEXPORT void JNICALL Java_SodiumTest_decryptData(JNIEnv *env, jclass clazz, jobject bbufData, jobject bbufRecipientPrivate, jobject bbufSenderPublic, jobject bbufNonce, jobject bbufOutput) {
	struct JCL_buffer bufData, bufNonce, bufRecipientPrivate, bufSenderPublic, bufOutput;
	JCL_init_buffer(env, &bufData, bbufData);
	JCL_init_buffer(env, &bufNonce, bbufNonce);
	JCL_init_buffer(env, &bufRecipientPrivate, bbufRecipientPrivate);
	JCL_init_buffer(env, &bufSenderPublic, bbufSenderPublic);
	JCL_init_buffer(env, &bufOutput, bbufOutput);

	jbyte temp_encrypted[MAX_MSG_SIZE];
	jbyte temp_plain[MAX_MSG_SIZE];
	int rc;
	int length = bufData.limit - crypto_box_BOXZEROBYTES;

	if(length + crypto_box_BOXZEROBYTES >= MAX_MSG_SIZE) {
		JCL_ThrowException(env, "java/lang/Exception", "Data too long");
		return;
	}

	memset(temp_encrypted, '\0', crypto_box_BOXZEROBYTES);
	memcpy(temp_encrypted + crypto_box_BOXZEROBYTES, bufData.ptr, length);

	rc = crypto_box_open(temp_plain, temp_encrypted, crypto_box_BOXZEROBYTES + length, bufNonce.ptr, bufSenderPublic.ptr, bufRecipientPrivate.ptr);
	if (rc != 0) {
		JCL_ThrowException(env, "java/lang/Exception", "Decryption failed (return code)");
		return;
	}

	if (is_zero(temp_plain, crypto_box_ZEROBYTES) != 0) {
		JCL_ThrowException(env, "java/lang/Exception", "Decryption failed (zero bytes)");
		return;
	}

	if (crypto_box_BOXZEROBYTES + length - crypto_box_ZEROBYTES > bufOutput.limit) {
		JCL_ThrowException(env, "java/lang/Exception", "Output buffer too short");
		return;
	}

	memcpy(bufOutput.ptr, temp_plain + crypto_box_ZEROBYTES, crypto_box_BOXZEROBYTES + length - crypto_box_ZEROBYTES);

	JCL_release_buffer(env, &bufData, bbufData, JNI_ABORT);
	JCL_release_buffer(env, &bufNonce, bbufNonce, JNI_ABORT);
	JCL_release_buffer(env, &bufRecipientPrivate, bbufRecipientPrivate, JNI_ABORT);
	JCL_release_buffer(env, &bufSenderPublic, bbufSenderPublic, JNI_ABORT);
	JCL_release_buffer(env, &bufOutput, bbufOutput, 0);
}

JNIEXPORT void JNICALL Java_SodiumTest_squareMatrix(JNIEnv *env, jclass clazz, jint size, jfloatArray matrix1, jfloatArray matrix2) {
	jfloat *matrixOrig = (*env)->GetFloatArrayElements(env, matrix1, NULL);
	jfloat *matrixNew = (*env)->GetFloatArrayElements(env, matrix2, NULL);

	int i, j, k;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			matrixNew[i*size + j] = 0.0f;
			for (k = 0; k < size; k++)
				matrixNew[i*size + j] += matrixOrig[i*size + k] * matrixOrig[k*size + j];
		}
	}

	(*env)->ReleaseFloatArrayElements(env, matrix1, matrixOrig, JNI_ABORT);
	(*env)->ReleaseFloatArrayElements(env, matrix2, matrixNew, 0);
}

JNIEXPORT jboolean JNICALL Java_SodiumTest_readAccess(JNIEnv *env, jclass clazz, jstring jPath) {
	const char *cPath = (*env)->GetStringUTFChars(env, jPath, NULL);
	int ret = access(cPath, R_OK);
	(*env)->ReleaseStringUTFChars(env, jPath, cPath);

	return ret == 0;
}

