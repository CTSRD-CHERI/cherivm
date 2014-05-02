#include "guest.h"

#define TEST_START(name)    printf("[SANDBOX: %s... ", name)
#define TEST_PASSED			printf("passed]\n")
#define TEST_FAILED			printf("FAILED]\n")
#define TEST_FAILED_REASON(reason)			printf("FAILED (%s)]\n", reason)

void cherijni_runTests(JNIEnv *env) {
	printf("[SANDBOX: Running tests...]\n");

	TEST_START("GetVersion");
	if ((*env)->GetVersion && ((*env)->GetVersion(env) == JNI_VERSION_1_6))
		TEST_PASSED;
	else
		TEST_FAILED;

	TEST_START("FindClass (non-existent)");
	if ((*env)->FindClass && ((*env)->FindClass(env, "com/wrong/GarbageClass") == NULL))
		TEST_PASSED;
	else
		TEST_FAILED;

	TEST_START("FindClass (correct)");
	// lookup a class that will be loaded by now (not even Integer will)
	if ((*env)->FindClass && ((*env)->FindClass(env, "java/lang/VMThrowable") != NULL))
		TEST_PASSED;
	else
		TEST_FAILED;

	TEST_START("IsInstanceOf");
	if ((*env)->FindClass && (*env)->IsInstanceOf &&
		(*env)->IsInstanceOf(env,
			(*env)->FindClass(env, "java/lang/VMThrowable"),
			(*env)->FindClass(env, "java/lang/Class")))
		TEST_PASSED;
	else
		TEST_FAILED;

	TEST_START("Objects are cached");
	if ((*env)->FindClass) {
		jclass obj1 = (*env)->FindClass(env, "java/lang/VMThrowable");
		jclass obj2 = (*env)->FindClass(env, "java/lang/VMThrowable");
		if (obj1 != NULL && obj1 == obj2)
			TEST_PASSED;
		else
			TEST_FAILED;
	} else
		TEST_FAILED;

	TEST_START("GetFieldID (legitimate)");
	if ((*env)->FindClass && (*env)->GetFieldID) {
		jclass clazz = (*env)->FindClass(env, "gnu/java/nio/VMChannel");
		if (clazz == NULL)
			TEST_FAILED;
		else {
			jfieldID field = (*env)->GetFieldID(env, clazz, "kind", "Lgnu/java/nio/VMChannel$Kind;");
			if (field != NULL)
				TEST_PASSED;
			else
				TEST_FAILED;
		}
	} else
		TEST_FAILED;

	TEST_START("GetFieldID (illegitimate)");
	if ((*env)->FindClass && (*env)->GetFieldID) {
		jclass clazz = (*env)->FindClass(env, "java/lang/VMThrowable");
		if (clazz == NULL)
			TEST_FAILED;
		else {
			/*
			 * Try to access a private field inside an unrelated class.
			 * Context check should prevent this!
			 */
			jfieldID field = (*env)->GetFieldID(env, clazz, "backtrace", "Ljava/lang/Object;");
			if (field == NULL)
				TEST_PASSED;
			else
				TEST_FAILED;
		}
	} else
		TEST_FAILED;

	TEST_START("StringUTF");
	if ((*env)->NewStringUTF && (*env)->GetStringUTFLength && (*env)->GetStringUTFChars && (*env)->ReleaseStringUTFChars) {
		jstring str = (*env)->NewStringUTF(env, "Hello, CheriJNI!!!");
		if (str == NULL)
			TEST_FAILED;
		else {
			jboolean isCopy;
			const char *buf = (*env)->GetStringUTFChars(env, str, &isCopy);
			if (buf == NULL || isCopy == JNI_FALSE || strcmp("Hello, CheriJNI!!!", buf) != 0)
				TEST_FAILED;
			else {
				(*env)->ReleaseStringUTFChars(env, str, buf);
				TEST_PASSED;
			}
		}
	} else
		TEST_FAILED;

	TEST_START("LocalRefs are invalidated");
	if ((*env)->FindClass && (*env)->PushLocalFrame && (*env)->PopLocalFrame && (*env)->NewStringUTF && (*env)->GetStringUTFLength) {
		int success = (*env)->PushLocalFrame(env, 16);
		if (success < 0)
			TEST_FAILED_REASON("couldn't create local frame");
		else {
			jobject str = (*env)->NewStringUTF(env, "Testing...");
			if (str == NULL)
				TEST_FAILED_REASON("couldn't create string");
			else {
				(*env)->PopLocalFrame(env, NULL);
				if ((*env)->GetStringUTFLength(env, str) > 0)
					TEST_FAILED_REASON("old ref accepted");
				else
					TEST_PASSED;
			}
		}
	} else
		TEST_FAILED;

	TEST_START("Remaining LocalRefs are not invalidated");
	if ((*env)->FindClass && (*env)->PushLocalFrame && (*env)->PopLocalFrame && (*env)->NewLocalRef && (*env)->NewStringUTF && (*env)->GetStringUTFLength) {
		jobject str = (*env)->NewStringUTF(env, "Testing...");
		if (str == NULL)
			TEST_FAILED_REASON("couldn't create string");
		else {
			int success = (*env)->PushLocalFrame(env, 16);
			if (success < 0)
				TEST_FAILED_REASON("couldn't create local frame");
			else {
				jobject str2 = (*env)->NewLocalRef(env, str2);
				(*env)->PopLocalFrame(env, NULL);
				if ((*env)->GetStringUTFLength(env, str) == 0)
					TEST_FAILED_REASON("old ref not accepted");
				else
					TEST_PASSED;
			}
		}
	} else
		TEST_FAILED;

	(*env)->ExceptionClear(env);
	printf("[SANDBOX: Finished running tests...]\n");
}
