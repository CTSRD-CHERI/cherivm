#include "guest.h"

#define TEST_START(name)    printf("[SANDBOX: %s... ", name)
#define TEST_PASSED			printf("passed]\n")
#define TEST_FAILED			printf("FAILED]\n")
#define TEST_FAILED_DEBUG   printf("FAILED (res=%d)]\n", temp)

void cherijni_runTests(JNIEnv *env) {
	printf("[SANDBOX: Running tests...]\n");
	uintptr_t temp = 1234;

	TEST_START("GetVersion");
	if ((*env)->GetVersion && ((temp = (*env)->GetVersion(env)) == JNI_VERSION_1_6))
		TEST_PASSED;
	else
		TEST_FAILED_DEBUG;

	printf("[SANDBOX: Finished running tests...]\n");
}
