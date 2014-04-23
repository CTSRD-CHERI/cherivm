#include "guest.h"

#define FNTYPE_VOID       1
#define FNTYPE_PRIMITIVE  2
#define FNTYPE_OBJECT     3

typedef struct method_entry {
	char *name;
	void *func;
	int type;
} methodEntry;

extern methodEntry cherijni_MethodList[];

struct cheri_object cherijni_SystemObject;

typedef jint (*fn_init)(JavaVM*, void*);
typedef void (*fn_void)(JNIEnv*, register_t, register_t, register_t, register_t, register_t, register_t);
typedef register_t (*fn_prim)(JNIEnv*, register_t, register_t, register_t, register_t, register_t, register_t);

static char* cherijni_extractHostString(__capability char* str_cap) {
	unsigned int i;
	size_t str_len = cheri_getlen(str_cap);
	char *buf = malloc(sizeof(char) * str_len);
	for (i = 0; i < str_len; i++)
		buf[i] = str_cap[i];
	return buf;
}

static void* cherijni_methodLookup(char *name) {
	methodEntry* entry = cherijni_MethodList;
	while (entry->name) {
		if (!strcmp(name, entry->name))
			return entry;
		entry++;
	}
	return NULL;
}

register_t cherijni_invoke(u_int op,
                           register_t a1, register_t a2, register_t a3,
                           register_t a4, register_t a5, register_t a6,
                           register_t a7, struct cheri_object system_object,
                           __capability void *c1, __capability void *c2,
                           __capability void *c3, __capability void *c4,
                           __capability void *c5, __capability void *c6) {

	cheri_system_setup(system_object);
	cherijni_SystemObject = system_object;

	if (op == CHERIJNI_METHOD_LOOKUP) {

		/*
		 * Find method by its name stored in $c5
		 */

		char *method_name = cherijni_extractHostString(c1);
		void *method_ptr = cherijni_methodLookup(method_name);
		return (register_t) method_ptr;

	} else if (op == CHERIJNI_METHOD_ONLOAD_ONUNLOAD) {

		/*
		 * Run JNI_OnLoad or JNI_OnUnload, pointer to the relevant
		 * method list entry is provided in $a1
		 */

		fn_init func = (fn_init) ((methodEntry*) a1)->func;
		jint result = func(cherijni_getJavaVM(), NULL);
		return (register_t) result;

	} else if (op == CHERIJNI_METHOD_RUN) {

		/*
		 * Run arbitrary method with JNI call convention.
		 * Pointer to the method list entry is in $a1
		 */

		methodEntry *entry = (methodEntry*) a1;
		JNIEnv *env = cherijni_getJNIEnv(&c1);
		register_t result;

		if (entry->type == FNTYPE_VOID) {

			printf("[SANDBOX: invoke method (void) %s]\n", entry->name);
			((fn_void) entry->func)(env, a2, a3, a4, a5, a6, a7);
			result = 0;

		} else if (entry->type == FNTYPE_PRIMITIVE) {

			printf("[SANDBOX: invoke method (prim) %s]\n", entry->name);
			result = ((fn_prim) entry->func)(env, a2, a3, a4, a5, a6, a7);

		} else if (entry->type == FNTYPE_OBJECT) {

			printf("[SANDBOX: invoke method (obj) %s]\n", entry->name);
			result = (-1);

		} else
			result = (-1);

		printf("[SANDBOX: returning %p]\n", (void*) result);
		cherijni_destroyJNIEnv(env);
		return result;

//	} else if (op == CHERIJNI_METHOD_TEST) {
//
//		JNIEnv *env = cherijni_getJNIEnv();
//		cherijni_runTests(env);
//		cherijni_destroyJNIEnv(env);
//
//		return 0;

	} else
		return (-1);
}
