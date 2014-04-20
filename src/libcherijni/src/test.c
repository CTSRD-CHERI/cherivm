#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

#define TEST_START(name)    printf("[SANDBOX: %s... ", name)
#define TEST_PASSED			printf("passed]\n")
#define TEST_FAILED			printf("FAILED]\n")

void cherijni_runTests(JNIEnv *env) {
	printf("[SANDBOX: Running tests...]\n");

	TEST_START("GetVersion");
	if ((*env)->GetVersion && (*env)->GetVersion(env) == JNI_VERSION_1_6)
		TEST_PASSED;
	else
		TEST_FAILED;

	printf("[SANDBOX: Finished running tests...]\n");
}
