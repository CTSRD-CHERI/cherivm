#include <jni.h>

#define UNUSED	__attribute__ ((__unused__))

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{NULL, NULL, 0}
};
