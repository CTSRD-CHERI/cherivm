#include "guest.h"

#define hostInvoke_name(name)  (CHERIJNI_JNIEnv_ ## name)

static jint GetVersion(JNIEnv *env) {
	return (jint) hostInvoke_0_0(cheri_invoke_prim, GetVersion);
}

static jclass FindClass(JNIEnv *env, const char *className) {
	/*
	 * Interesting: no point in passing the name as a read-only capability
	 * pass it as a pointer with the default capability
	 */
	__capability void *result = hostInvoke_0_1(cheri_invoke_cap, FindClass, cap_string(className));
	return (jclass) cherijni_jobject_store(result);
}

static jint ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
	register_t res = hostInvoke_0_2(cheri_invoke_prim, ThrowNew, get_cap(clazz), cap_string(msg));
	return (jint) res;
}

static jthrowable ExceptionOccurred(JNIEnv *env) {
	__capability void *result = hostInvoke_0_0(cheri_invoke_cap, ExceptionOccurred);
	return (jthrowable) cherijni_jobject_store(result);
}

static void ExceptionDescribe(JNIEnv *env) {
	check_cheri_fail_void(hostInvoke_0_0(cheri_invoke_prim, ExceptionDescribe));
}

static void ExceptionClear(JNIEnv *env) {
	check_cheri_fail_void(hostInvoke_0_0(cheri_invoke_prim, ExceptionClear));
}

static void DeleteLocalRef(JNIEnv *env, jobject localRef) {
	check_cheri_fail_void(hostInvoke_0_1(cheri_invoke_prim, ExceptionClear, get_cap(localRef)));
}

static jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
	register_t res = hostInvoke_0_2(cheri_invoke_prim, IsInstanceOf, get_cap(obj), get_cap(clazz));
	check_cheri_fail(res, JNI_FALSE);
	return res;
}

static jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	__capability void *result = hostInvoke_0_3(cheri_invoke_cap, GetMethodID, get_cap(clazz), cap_string(name), cap_string(sig));
	return (jmethodID) cherijni_jmethodID_store(result, sig);
}

#define VIRTUAL_METHOD_COMMON                                                                                                        \
	register_t args_prim[] = { 0, 0, 0, 0, 0, 0, 0 };                                                                                \
	__capability void *args_cap[] = { CNULL, CNULL, CNULL };                                                                         \
	size_t args_prim_ready = 0, args_cap_ready = 0;                                                                                  \
	va_list native_args;                                                                                                             \
																																	 \
	cherijni_objtype_jmethodID *mid_struct = (cherijni_objtype_jmethodID *) mid;                                                     \
	va_start(native_args, mid);                                                                                                      \
	scanSignature(mid_struct->sig,                                                                                                   \
	/* single primitives */ { args_prim[args_prim_ready++] = va_arg(native_args, register_t); },                                     \
	/* double primitives */ { args_prim[args_prim_ready++] = va_arg(native_args, register_t); },                                     \
	/* objects           */ { jobject obj = (jobject) va_arg(native_args, register_t); args_cap[args_cap_ready++] = get_cap(obj); }, \
	/* return values     */ { }, { }, { }, { });                                                                                     \
	va_end(native_args);

#define VIRTUAL_METHOD(TYPE, jtype)                                                                                \
	static jtype Call##TYPE##Method(JNIEnv *env, jobject obj, jmethodID mid, ...) {                                \
		VIRTUAL_METHOD_COMMON                                                                                      \
		return (jtype) hostInvoke_7_5(cheri_invoke_prim, Call##TYPE##Method,                                       \
				args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5], args_prim[6],  \
				get_cap(obj), get_cap(mid), args_cap[0], args_cap[1], args_cap[2]);                                \
	}

static jobject CallObjectMethod(JNIEnv *env, jobject obj, jmethodID mid, ...) {
	VIRTUAL_METHOD_COMMON
	__capability void *result = hostInvoke_7_5(cheri_invoke_cap, CallObjectMethod,
			args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5], args_prim[6],
			get_cap(obj), get_cap(mid), args_cap[0], args_cap[1], args_cap[2]);
	return (jobject) cherijni_jobject_store(result);
}

#define CALL_METHOD(access)        \
access##_METHOD(Boolean, jboolean) \
access##_METHOD(Byte, jbyte)       \
access##_METHOD(Char, jchar)       \
access##_METHOD(Short, jshort)     \
access##_METHOD(Int, jint)         \
access##_METHOD(Long, jlong)       \
access##_METHOD(Float, jfloat)     \
access##_METHOD(Double, jdouble)   \
access##_METHOD(Void, void)

CALL_METHOD(VIRTUAL)

static jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	__capability void *result = hostInvoke_0_3(cheri_invoke_cap, GetFieldID, get_cap(clazz), cap_string(name), cap_string(sig));
	return (jfieldID) cherijni_jfieldID_store(result);
}

static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
	__capability void *result = hostInvoke_0_3(cheri_invoke_cap, GetStaticMethodID, get_cap(clazz), cap_string(name), cap_string(sig));
	return (jmethodID) cherijni_jmethodID_store(result, sig);
}

static jstring NewStringUTF(JNIEnv *env, const char *bytes) {
	__capability void *result = hostInvoke_0_1(cheri_invoke_cap, NewStringUTF, cap_string(bytes));
	return (jstring) cherijni_jobject_store(result);
}

static jsize GetStringUTFLength(JNIEnv *env, jstring string) {
	register_t result = hostInvoke_0_1(cheri_invoke_prim, GetStringUTFLength, get_cap(string));
	check_cheri_fail(result, 0);
	return result;
}

static const char *GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy) {
	jsize str_length = (*env)->GetStringUTFLength(env, string);
	char *str_buffer = (char *) malloc(str_length + 1);
	register_t result = hostInvoke_0_2(cheri_invoke_prim, GetStringUTFChars, get_cap(string), cap_buffer(str_buffer, str_length + 1));
	check_cheri_fail_extra(result, NULL, free(str_buffer));

	if (isCopy != NULL)
		*isCopy = JNI_TRUE;
	return str_buffer;
}

static void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *utf) {
	if (utf != NULL)
		free((void*)utf);
}

static jsize GetArrayLength(JNIEnv *env, jarray array) {
	return (jsize) hostInvoke_0_1(cheri_invoke_prim, GetArrayLength, get_cap(array));
}

#define GET_ARRAY_ELEMENTS(TYPE, jtype, ctype) \
	static jtype *Get##TYPE##ArrayElements(JNIEnv *env, jtype##Array array, jboolean *isCopy) { \
		jsize length = (*env)->GetArrayLength(env, array); \
		size_t buffer_size = length * sizeof(jtype); \
		jtype *buffer = (jtype*) malloc(buffer_size); \
		register_t result = hostInvoke_0_2(cheri_invoke_prim, Get##TYPE##ArrayElements, get_cap(array), cap_buffer(buffer, buffer_size)); \
		check_cheri_fail_extra(result, NULL, free(buffer)); \
		\
		if (isCopy != NULL) \
			*isCopy = JNI_TRUE; \
		return buffer; \
	} \
	\
	static void Release##TYPE##ArrayElements(JNIEnv *env, jtype##Array array, jtype *elems, jint mode) { \
		if (mode != JNI_ABORT) { \
			jsize length = (*env)->GetArrayLength(env, array); \
			(*env)->Set##TYPE##ArrayRegion(env, array, 0, length, elems); \
		} \
		\
		if (mode != JNI_COMMIT) \
			free(elems);\
	}

#define ARRAY_METHOD(op)   \
op(Boolean, jboolean, 'Z') \
op(Byte, jbyte, 'B')       \
op(Char, jchar, 'C')       \
op(Short, jshort, 'S')     \
op(Int, jint, 'I')         \
op(Long, jlong, 'J')       \
op(Float, jfloat, 'F')     \
op(Double, jdouble, 'D')

ARRAY_METHOD(GET_ARRAY_ELEMENTS)

static void *GetDirectBufferAddress(JNIEnv *env, jobject buf) {
	__capability void *result = hostInvoke_0_1(cheri_invoke_cap, GetDirectBufferAddress, get_cap(buf));
	if (result == CNULL)
		return NULL;
	else {
		printf("GetDirectBufferAddress returned: \n");
		CHERI_CAP_PRINT(result);
		printf("warning: returning NULL instead!\n");
		return NULL;
	}
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
		DeleteLocalRef,
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
		CallObjectMethod,
		NULL, // CallObjectMethodV,
		NULL, // CallObjectMethodA,
		CallBooleanMethod,
		NULL, // CallBooleanMethodV,
		NULL, // CallBooleanMethodA,
		CallByteMethod,
		NULL, // CallByteMethodV,
		NULL, // CallByteMethodA,
		CallCharMethod,
		NULL, // CallCharMethodV,
		NULL, // CallCharMethodA,
		CallShortMethod,
		NULL, // CallShortMethodV,
		NULL, // CallShortMethodA,
		CallIntMethod,
		NULL, // CallIntMethodV,
		NULL, // CallIntMethodA,
		CallLongMethod,
		NULL, // CallLongMethodV,
		NULL, // CallLongMethodA,
		CallFloatMethod,
		NULL, // CallFloatMethodV,
		NULL, // CallFloatMethodA,
		CallDoubleMethod,
		NULL, // CallDoubleMethodV,
		NULL, // CallDoubleMethodA,
		CallVoidMethod,
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
		GetArrayLength,
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
		GetBooleanArrayElements,
		GetByteArrayElements,
		GetCharArrayElements,
		GetShortArrayElements,
		GetIntArrayElements,
		GetLongArrayElements,
		GetFloatArrayElements,
		GetDoubleArrayElements,
		ReleaseBooleanArrayElements,
		ReleaseByteArrayElements,
		ReleaseCharArrayElements,
		ReleaseShortArrayElements,
		ReleaseIntArrayElements,
		ReleaseLongArrayElements,
		ReleaseFloatArrayElements,
		ReleaseDoubleArrayElements,
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
		GetDirectBufferAddress,
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
