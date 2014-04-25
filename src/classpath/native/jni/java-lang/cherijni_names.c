#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

extern jlong Java_java_lang_VMDouble_doubleToRawLongBits (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble doubleValue);
extern void Java_java_lang_VMDouble_initIDs (JNIEnv * env, jclass cls __attribute__ ((__unused__)));
extern jdouble Java_java_lang_VMDouble_longBitsToDouble (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jlong longValue);
extern jdouble Java_java_lang_VMDouble_parseDouble (JNIEnv * env, jclass cls __attribute__ ((__unused__)), jstring str);
extern jstring Java_java_lang_VMDouble_toString (JNIEnv * env, jclass cls __attribute__ ((__unused__)), jdouble value, jboolean isFloat);
extern jint Java_java_lang_VMFloat_floatToRawIntBits (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jfloat value);
extern jfloat Java_java_lang_VMFloat_intBitsToFloat (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jint bits);
extern jdouble Java_java_lang_VMMath_IEEEremainder (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_acos (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_asin (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_atan (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_atan2 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble y, jdouble x);
extern jdouble Java_java_lang_VMMath_cbrt (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_ceil (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_cos (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_cosh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_exp (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_expm1 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_floor (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_hypot (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_log (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_log10 (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_log1p (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_pow (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x, jdouble y);
extern jdouble Java_java_lang_VMMath_rint (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sin (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sinh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_sqrt (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_tan (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern jdouble Java_java_lang_VMMath_tanh (JNIEnv * env __attribute__ ((__unused__)), jclass cls __attribute__ ((__unused__)), jdouble x);
extern void Java_java_lang_VMProcess_nativeKill (JNIEnv * env, jclass clazz, jlong pid);
extern jboolean Java_java_lang_VMProcess_nativeReap (JNIEnv * env, jclass clazz);
extern void Java_java_lang_VMProcess_nativeSpawn (JNIEnv * env, jobject this, jobjectArray cmdArray, jobjectArray envArray, jobject dirFile, jboolean redirect);
extern jlong Java_java_lang_VMSystem_currentTimeMillis (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)));
extern jobject Java_java_lang_VMSystem_environ (JNIEnv *env, jclass klass __attribute__((__unused__)));
extern jstring Java_java_lang_VMSystem_getenv (JNIEnv * env, jclass klass __attribute__ ((__unused__)), jstring jname);
extern jlong Java_java_lang_VMSystem_nanoTime (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)));
extern void Java_java_lang_VMSystem_setErr (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);
extern void Java_java_lang_VMSystem_setIn (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);
extern void Java_java_lang_VMSystem_setOut (JNIEnv * env, jclass thisClass __attribute__ ((__unused__)), jobject obj);

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{ "Java_java_lang_VMDouble_doubleToRawLongBits", &Java_java_lang_VMDouble_doubleToRawLongBits, 2 },
	{ "Java_java_lang_VMDouble_initIDs", &Java_java_lang_VMDouble_initIDs, 1 },
	{ "Java_java_lang_VMDouble_longBitsToDouble", &Java_java_lang_VMDouble_longBitsToDouble, 2 },
	{ "Java_java_lang_VMDouble_parseDouble", &Java_java_lang_VMDouble_parseDouble, 2 },
	{ "Java_java_lang_VMDouble_toString", &Java_java_lang_VMDouble_toString, 3 },
	{ "Java_java_lang_VMFloat_floatToRawIntBits", &Java_java_lang_VMFloat_floatToRawIntBits, 2 },
	{ "Java_java_lang_VMFloat_intBitsToFloat", &Java_java_lang_VMFloat_intBitsToFloat, 2 },
	{ "Java_java_lang_VMMath_IEEEremainder", &Java_java_lang_VMMath_IEEEremainder, 2 },
	{ "Java_java_lang_VMMath_acos", &Java_java_lang_VMMath_acos, 2 },
	{ "Java_java_lang_VMMath_asin", &Java_java_lang_VMMath_asin, 2 },
	{ "Java_java_lang_VMMath_atan", &Java_java_lang_VMMath_atan, 2 },
	{ "Java_java_lang_VMMath_atan2", &Java_java_lang_VMMath_atan2, 2 },
	{ "Java_java_lang_VMMath_cbrt", &Java_java_lang_VMMath_cbrt, 2 },
	{ "Java_java_lang_VMMath_ceil", &Java_java_lang_VMMath_ceil, 2 },
	{ "Java_java_lang_VMMath_cos", &Java_java_lang_VMMath_cos, 2 },
	{ "Java_java_lang_VMMath_cosh", &Java_java_lang_VMMath_cosh, 2 },
	{ "Java_java_lang_VMMath_exp", &Java_java_lang_VMMath_exp, 2 },
	{ "Java_java_lang_VMMath_expm1", &Java_java_lang_VMMath_expm1, 2 },
	{ "Java_java_lang_VMMath_floor", &Java_java_lang_VMMath_floor, 2 },
	{ "Java_java_lang_VMMath_hypot", &Java_java_lang_VMMath_hypot, 2 },
	{ "Java_java_lang_VMMath_log", &Java_java_lang_VMMath_log, 2 },
	{ "Java_java_lang_VMMath_log10", &Java_java_lang_VMMath_log10, 2 },
	{ "Java_java_lang_VMMath_log1p", &Java_java_lang_VMMath_log1p, 2 },
	{ "Java_java_lang_VMMath_pow", &Java_java_lang_VMMath_pow, 2 },
	{ "Java_java_lang_VMMath_rint", &Java_java_lang_VMMath_rint, 2 },
	{ "Java_java_lang_VMMath_sin", &Java_java_lang_VMMath_sin, 2 },
	{ "Java_java_lang_VMMath_sinh", &Java_java_lang_VMMath_sinh, 2 },
	{ "Java_java_lang_VMMath_sqrt", &Java_java_lang_VMMath_sqrt, 2 },
	{ "Java_java_lang_VMMath_tan", &Java_java_lang_VMMath_tan, 2 },
	{ "Java_java_lang_VMMath_tanh", &Java_java_lang_VMMath_tanh, 2 },
	{ "Java_java_lang_VMProcess_nativeKill", &Java_java_lang_VMProcess_nativeKill, 1 },
	{ "Java_java_lang_VMProcess_nativeReap", &Java_java_lang_VMProcess_nativeReap, 2 },
	{ "Java_java_lang_VMProcess_nativeSpawn", &Java_java_lang_VMProcess_nativeSpawn, 1 },
	{ "Java_java_lang_VMSystem_currentTimeMillis", &Java_java_lang_VMSystem_currentTimeMillis, 2 },
	{ "Java_java_lang_VMSystem_environ", &Java_java_lang_VMSystem_environ, 3 },
	{ "Java_java_lang_VMSystem_getenv", &Java_java_lang_VMSystem_getenv, 3 },
	{ "Java_java_lang_VMSystem_nanoTime", &Java_java_lang_VMSystem_nanoTime, 2 },
	{ "Java_java_lang_VMSystem_setErr", &Java_java_lang_VMSystem_setErr, 1 },
	{ "Java_java_lang_VMSystem_setIn", &Java_java_lang_VMSystem_setIn, 1 },
	{ "Java_java_lang_VMSystem_setOut", &Java_java_lang_VMSystem_setOut, 1 },
	{NULL, NULL, 0}
};
