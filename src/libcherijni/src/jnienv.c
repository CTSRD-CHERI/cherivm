#include "guest.h"

#define CNULL		(cheri_zerocap())

static __capability void *cherijni_output;

#define SendObject(obj) (cherijni_obj_loadcap(env, obj))
#define COutput (cheri_ptrperm(&cherijni_output, sizeof(__capability void*), CHERI_PERM_STORE | CHERI_PERM_STORE_CAP))

//#define hostInvoke_2_2(name, env, a1, a2, c1, c2) \
//	(cheri_invoke(cherijni_SystemObject, \
//	    CHERIJNI_JNIEnv_ ## name, \
//	    a1, a2, 0, 0, 0, 0, 0, \
//	    cheri_zerocap(), COutput, \
//	    c1,              c2,              cheri_zerocap(), \
//	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap()))
//#define hostInvoke_0_2(name, env, c1, c2)  hostInvoke_2_2(name, env, 0, 0, c1, c2)
//#define hostInvoke_1_1(name, env, a1, c1)  hostInvoke_2_2(name, env, a1, 0, c1, cheri_zerocap())
//#define hostInvoke_0_0(name, env)          hostInvoke_1_1(name, env, 0, cheri_zerocap())
//#define hostInvoke_1_0(name, env, a1)      hostInvoke_1_1(name, env, a1, cheri_zerocap())
//#define hostInvoke_0_1(name, env, c1)      hostInvoke_1_1(name, env, 0, c1)

#define hostInvoke_2(name, env, a1, a2) \
	(cheri_invoke(cherijni_SystemObject, \
	    CHERIJNI_JNIEnv_ ## name, \
	    a1, a2, 0, 0, 0, 0, 0, \
	    cheri_getdefault(), \
	    *((__capability void**) ((*env)->cherijni_context)), \
		COutput, \
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap()))
#define hostInvoke_1(name, env, a1)     hostInvoke_2(name, env, a1, 0)
#define hostInvoke_0(name, env)         hostInvoke_1(name, env, 0)

static jint GetVersion(JNIEnv *env) {
	return (jint) hostInvoke_0(GetVersion, env);
}

static jclass FindClass(JNIEnv *env, const char *className) {
	/*
	 * Interesting: no point in passing the name as a read-only capability
	 * pass it as a pointer with the default capability
	 */
	if (hostInvoke_1(FindClass, env, (register_t) className) == 0) {
		return (jclass) cherijni_obj_storecap(env, cherijni_output);
	} else {
		printf("[SANDBOX ERROR: call to FindClass failed]\n");
		return NULL;
	}
}

static jthrowable ExceptionOccured(JNIEnv *env) {
	printf("[JNIEnv %s stub]\n", __func__);
	return NULL;
}

static void ExceptionClear(JNIEnv *env) {
	printf("[JNIEnv %s stub]\n", __func__);
}

static jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
	register_t result = hostInvoke_2(IsInstanceOf, env, obj, clazz);
	if (result < 0) {
		printf("[SANDBOX ERROR: call to IsInstanceOf failed]\n");
		return JNI_FALSE;
	} else
		return result;
}

static jint ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
	printf("[JNIEnv %s stub]\n", __func__);
	return (-1);
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
		NULL, // ThrowNew,
		NULL, // ExceptionOccurred,
		NULL, // ExceptionDescribe,
		NULL, // ExceptionClear,
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
		NULL, // GetMethodID,
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
		NULL, // GetFieldID,
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
		NULL, // GetStaticMethodID,
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
		NULL, // NewStringUTF,
		NULL, // GetStringUTFLength,
		NULL, // GetStringUTFChars,
		NULL, // ReleaseStringUTFChars,
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

JNIEnv *cherijni_getJNIEnv(__capability void **context) {
	void *mem = malloc(sizeof(JNIEnv) + sizeof(struct _JNINativeInterface));

	JNIEnv *ppEnv = (JNIEnv*) mem;
	struct _JNINativeInterface *pEnv = (struct _JNINativeInterface*) (ppEnv + 1);

	memcpy(pEnv, &cherijni_JNIEnv_struct, sizeof(struct _JNINativeInterface));
	pEnv->cherijni_context = context;
	cherijni_obj_init(pEnv);

	*ppEnv = pEnv;
	return ppEnv;
}

void cherijni_destroyJNIEnv(JNIEnv *ppEnv) {
	free(ppEnv);
}
