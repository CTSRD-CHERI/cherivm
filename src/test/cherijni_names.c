#include <cherijni.h>

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

extern void cherijni_runTests(JNIEnv *env);

methodEntry cherijni_MethodList[] = {
	{"Java_gnu_java_nio_VMChannel_initIDs__", &cherijni_runTests, 1},
	{NULL, NULL, 0}
};
