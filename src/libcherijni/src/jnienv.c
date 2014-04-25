#include "guest.h"

#define hostInvoke_7(name, a1, a2, a3, a4, a5, a6, a7) \
	(cheri_invoke(cherijni_obj_system, \
	    CHERIJNI_JNIEnv_ ## name, \
	    a1, a2, a3, 0, 0, 0, 0, \
	    cheri_getdefault(), \
	    CNULL, \
		cap_output, \
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap()))
		/* *((__capability void**) ((*env)->cherijni_context)), \ */

static jint GetVersion(JNIEnv *env) {
	return (jint) hostInvoke_0(GetVersion);
}

static jclass FindClass(JNIEnv *env, const char *className) {
	/*
	 * Interesting: no point in passing the name as a read-only capability
	 * pass it as a pointer with the default capability
	 */
	register res = hostInvoke_1(FindClass, (register_t) className);
	check_cheri_fail(res, NULL);
	return_obj(jclass);
}

static jint ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
	register_t res = hostInvoke_2(ThrowNew, clazz, msg);
	check_cheri_fail(res, -1);
	return (jint) res;
}

static jthrowable ExceptionOccurred(JNIEnv *env) {
	register res = hostInvoke_0(ExceptionOccurred);
	check_cheri_fail(res, NULL);
	return_obj(jthrowable);
}

static void ExceptionDescribe(JNIEnv *env) {
	check_cheri_fail_void(hostInvoke_0(ExceptionDescribe))
}

static void ExceptionClear(JNIEnv *env) {
	check_cheri_fail_void(hostInvoke_0(ExceptionClear));
}

static jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
	register_t res = hostInvoke_2(IsInstanceOf, obj, clazz);
	check_cheri_fail(res, JNI_FALSE);
	return res;
}

static jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	register_t res = hostInvoke_3(GetMethodID, clazz, name, sig);
	check_cheri_fail(res, NULL);
	return_obj(jmethodID);
}

static jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	register_t res = hostInvoke_3(GetFieldID, clazz, name, sig);
	check_cheri_fail(res, NULL);
	return_obj(jfieldID);
}

static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	register_t res = hostInvoke_3(GetStaticMethodID, clazz, name, sig);
	check_cheri_fail(res, NULL);
	return_obj(jmethodID);
}

static jstring NewStringUTF(JNIEnv *env, const char *bytes) {
	register_t result = hostInvoke_1(NewStringUTF, bytes);
	check_cheri_fail(result, NULL);
	return_obj(jstring);
}

static jsize GetStringUTFLength(JNIEnv *env, jstring string) {
	register_t result = hostInvoke_1(GetStringUTFLength, string);
	check_cheri_fail(result, 0);
	return_obj(jstring);
}

static const char * GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy) {
	jsize str_length = (*env)->GetStringUTFLength(env, string);
	const char *str_buffer = (const char *) malloc(str_length + 1);
	register_t result = hostInvoke_2(GetStringUTFChars, string, str_buffer);
	check_cheri_fail(result, 0);

	if (isCopy != NULL)
		*isCopy = JNI_TRUE;
	return str_buffer;
}

static void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *utf) {
	free((void*)utf);
}

static struct _JNINativeInterface cherijni_JNIEnv_struct = {
		NULL,
		NULL,
		NULL,
		NULL,

		GetVersion,
		NULL, // DefineClass,
		FindClass,
		NULL, // FromReflectedMethod,
		NULL, // FromReflectedField,
		NULL, // ToReflectedMethod,
		NULL, // GetSuperclass,
		NULL, // IsAssignableFrom,
		NULL, // ToReflectedField,
		NULL, // Throw,
		ThrowNew,
		ExceptionOccurred,
		ExceptionDescribe,
		ExceptionClear,
		NULL, // FatalError,
		NULL, // PushLocalFrame,
		NULL, // PopLocalFrame,
		NULL, // NewGlobalRef,
		NULL, // DeleteGlobalRef,
		NULL, // DeleteLocalRef,
		NULL, // IsSameObject,
		NULL, // NewLocalRef,
		NULL, // EnsureLocalCapacity,
		NULL, // AllocObject,
		NULL, // NewObject,
		NULL, // NewObjectV,
		NULL, // NewObjectA,
		NULL, // GetObjectClass,
		IsInstanceOf,
		GetMethodID,
		NULL, // CallObjectMethod,
		NULL, // CallObjectMethodV,
		NULL, // CallObjectMethodA,
		NULL, // CallBooleanMethod,
		NULL, // CallBooleanMethodV,
		NULL, // CallBooleanMethodA,
		NULL, // CallByteMethod,
		NULL, // CallByteMethodV,
		NULL, // CallByteMethodA,
		NULL, // CallCharMethod,
		NULL, // CallCharMethodV,
		NULL, // CallCharMethodA,
		NULL, // CallShortMethod,
		NULL, // CallShortMethodV,
		NULL, // CallShortMethodA,
		NULL, // CallIntMethod,
		NULL, // CallIntMethodV,
		NULL, // CallIntMethodA,
		NULL, // CallLongMethod,
		NULL, // CallLongMethodV,
		NULL, // CallLongMethodA,
		NULL, // CallFloatMethod,
		NULL, // CallFloatMethodV,
		NULL, // CallFloatMethodA,
		NULL, // CallDoubleMethod,
		NULL, // CallDoubleMethodV,
		NULL, // CallDoubleMethodA,
		NULL, // CallVoidMethod,
		NULL, // CallVoidMethodV,
		NULL, // CallVoidMethodA,
		NULL, // CallNonvirtualObjectMethod,
		NULL, // CallNonvirtualObjectMethodV,
		NULL, // CallNonvirtualObjectMethodA,
		NULL, // CallNonvirtualBooleanMethod,
		NULL, // CallNonvirtualBooleanMethodV,
		NULL, // CallNonvirtualBooleanMethodA,
		NULL, // CallNonvirtualByteMethod,
		NULL, // CallNonvirtualByteMethodV,
		NULL, // CallNonvirtualByteMethodA,
		NULL, // CallNonvirtualCharMethod,
		NULL, // CallNonvirtualCharMethodV,
		NULL, // CallNonvirtualCharMethodA,
		NULL, // CallNonvirtualShortMethod,
		NULL, // CallNonvirtualShortMethodV,
		NULL, // CallNonvirtualShortMethodA,
		NULL, // CallNonvirtualIntMethod,
		NULL, // CallNonvirtualIntMethodV,
		NULL, // CallNonvirtualIntMethodA,
		NULL, // CallNonvirtualLongMethod,
		NULL, // CallNonvirtualLongMethodV,
		NULL, // CallNonvirtualLongMethodA,
		NULL, // CallNonvirtualFloatMethod,
		NULL, // CallNonvirtualFloatMethodV,
		NULL, // CallNonvirtualFloatMethodA,
		NULL, // CallNonvirtualDoubleMethod,
		NULL, // CallNonvirtualDoubleMethodV,
		NULL, // CallNonvirtualDoubleMethodA,
		NULL, // CallNonvirtualVoidMethod,
		NULL, // CallNonvirtualVoidMethodV,
		NULL, // CallNonvirtualVoidMethodA,
		GetFieldID,
		NULL, // GetObjectField,
		NULL, // GetBooleanField,
		NULL, // GetByteField,
		NULL, // GetCharField,
		NULL, // GetShortField,
		NULL, // GetIntField,
		NULL, // GetLongField,
		NULL, // GetFloatField,
		NULL, // GetDoubleField,
		NULL, // SetObjectField,
		NULL, // SetBooleanField,
		NULL, // SetByteField,
		NULL, // SetCharField,
		NULL, // SetShortField,
		NULL, // SetIntField,
		NULL, // SetLongField,
		NULL, // SetFloatField,
		NULL, // SetDoubleField,
		GetStaticMethodID,
		NULL, // CallStaticObjectMethod,
		NULL, // CallStaticObjectMethodV,
		NULL, // CallStaticObjectMethodA,
		NULL, // CallStaticBooleanMethod,
		NULL, // CallStaticBooleanMethodV,
		NULL, // CallStaticBooleanMethodA,
		NULL, // CallStaticByteMethod,
		NULL, // CallStaticByteMethodV,
		NULL, // CallStaticByteMethodA,
		NULL, // CallStaticCharMethod,
		NULL, // CallStaticCharMethodV,
		NULL, // CallStaticCharMethodA,
		NULL, // CallStaticShortMethod,
		NULL, // CallStaticShortMethodV,
		NULL, // CallStaticShortMethodA,
		NULL, // CallStaticIntMethod,
		NULL, // CallStaticIntMethodV,
		NULL, // CallStaticIntMethodA,
		NULL, // CallStaticLongMethod,
		NULL, // CallStaticLongMethodV,
		NULL, // CallStaticLongMethodA,
		NULL, // CallStaticFloatMethod,
		NULL, // CallStaticFloatMethodV,
		NULL, // CallStaticFloatMethodA,
		NULL, // CallStaticDoubleMethod,
		NULL, // CallStaticDoubleMethodV,
		NULL, // CallStaticDoubleMethodA,
		NULL, // CallStaticVoidMethod,
		NULL, // CallStaticVoidMethodV,
		NULL, // CallStaticVoidMethodA,
		NULL, // GetStaticFieldID,
		NULL, // GetStaticObjectField,
		NULL, // GetStaticBooleanField,
		NULL, // GetStaticByteField,
		NULL, // GetStaticCharField,
		NULL, // GetStaticShortField,
		NULL, // GetStaticIntField,
		NULL, // GetStaticLongField,
		NULL, // GetStaticFloatField,
		NULL, // GetStaticDoubleField,
		NULL, // SetStaticObjectField,
		NULL, // SetStaticBooleanField,
		NULL, // SetStaticByteField,
		NULL, // SetStaticCharField,
		NULL, // SetStaticShortField,
		NULL, // SetStaticIntField,
		NULL, // SetStaticLongField,
		NULL, // SetStaticFloatField,
		NULL, // SetStaticDoubleField,
		NULL, // NewString,
		NULL, // GetStringLength,
		NULL, // GetStringChars,
		NULL, // ReleaseStringChars,
		NewStringUTF,
		GetStringUTFLength,
		GetStringUTFChars,
		ReleaseStringUTFChars,
		NULL, // GetArrayLength,
		NULL, // NewObjectArray,
		NULL, // GetObjectArrayElement,
		NULL, // SetObjectArrayElement,
		NULL, // NewBooleanArray,
		NULL, // NewByteArray,
		NULL, // NewCharArray,
		NULL, // NewShortArray,
		NULL, // NewIntArray,
		NULL, // NewLongArray,
		NULL, // NewFloatArray,
		NULL, // NewDoubleArray,
		NULL, // GetBooleanArrayElements,
		NULL, // GetByteArrayElements,
		NULL, // GetCharArrayElements,
		NULL, // GetShortArrayElements,
		NULL, // GetIntArrayElements,
		NULL, // GetLongArrayElements,
		NULL, // GetFloatArrayElements,
		NULL, // GetDoubleArrayElements,
		NULL, // ReleaseBooleanArrayElements,
		NULL, // ReleaseByteArrayElements,
		NULL, // ReleaseCharArrayElements,
		NULL, // ReleaseShortArrayElements,
		NULL, // ReleaseIntArrayElements,
		NULL, // ReleaseLongArrayElements,
		NULL, // ReleaseFloatArrayElements,
		NULL, // ReleaseDoubleArrayElements,
		NULL, // GetBooleanArrayRegion,
		NULL, // GetByteArrayRegion,
		NULL, // GetCharArrayRegion,
		NULL, // GetShortArrayRegion,
		NULL, // GetIntArrayRegion,
		NULL, // GetLongArrayRegion,
		NULL, // GetFloatArrayRegion,
		NULL, // GetDoubleArrayRegion,
		NULL, // SetBooleanArrayRegion,
		NULL, // SetByteArrayRegion,
		NULL, // SetCharArrayRegion,
		NULL, // SetShortArrayRegion,
		NULL, // SetIntArrayRegion,
		NULL, // SetLongArrayRegion,
		NULL, // SetFloatArrayRegion,
		NULL, // SetDoubleArrayRegion,
		NULL, // RegisterNatives,
		NULL, // UnregisterNatives,
		NULL, // MonitorEnter,
		NULL, // MonitorExit,
		NULL, // GetJavaVM,
		NULL, // GetStringRegion,
		NULL, // GetStringUTFRegion,
		NULL, // GetPrimitiveArrayCritical,
		NULL, // ReleasePrimitiveArrayCritical,
		NULL, // GetStringCritical,
		NULL, // ReleaseStringCritical,
		NULL, // NewWeakGlobalRef,
		NULL, // DeleteWeakGlobalRef,
		NULL, // ExceptionCheck,
		NULL, // NewDirectByteBuffer,
		NULL, // GetDirectBufferAddress,
		NULL, // GetDirectBufferCapacity,
		NULL  // GetObjectRefType,
};
static JNIEnv cherijni_JNIEnv = &cherijni_JNIEnv_struct;

JNIEnv *cherijni_getJNIEnv(__capability void **context) {
//	void *mem = malloc(sizeof(JNIEnv) + sizeof(struct _JNINativeInterface));
//
//	JNIEnv *ppEnv = (JNIEnv*) mem;
//	struct _JNINativeInterface *pEnv = (struct _JNINativeInterface*) (ppEnv + 1);
//
//	memcpy(pEnv, &cherijni_JNIEnv_struct, sizeof(struct _JNINativeInterface));
//	pEnv->cherijni_context = context;
//
//	*ppEnv = pEnv;
//	return ppEnv;
	return &cherijni_JNIEnv;
}

void cherijni_destroyJNIEnv(JNIEnv *ppEnv) {
//	free(ppEnv);
}
