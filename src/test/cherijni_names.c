#include <cherijni.h>

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

methodEntry cherijni_MethodList[] = {
	{NULL, NULL}
};
