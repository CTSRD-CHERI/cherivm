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

struct cheri_object cherijni_obj_system;
__capability void *cherijni_output;

typedef jint (*fn_init)(JavaVM*, void*);
typedef register_t (*fn_jni)(JNIEnv*, __capability void *, register_t, register_t, register_t, register_t, register_t, register_t, __capability void*, __capability void*, __capability void*, __capability void*, __capability void*, __capability void*);
typedef jobject (*fn_jni_obj)(JNIEnv*, __capability void *, register_t, register_t, register_t, register_t, register_t, register_t, __capability void*, __capability void*, __capability void*, __capability void*, __capability void*, __capability void*);

char* cherijni_extractHostString(__capability char* str_cap) {
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

static jboolean ranTests = JNI_FALSE;

register_t cherijni_invoke(u_int op,
                           register_t a1, register_t a2, register_t a3,
                           register_t a4, register_t a5, register_t a6,
                           register_t a7,
                           __capability void *c1, __capability void *c2,
                           __capability void *c3, __capability void *c4,
                           __capability void *c5, __capability void *c6,
                           __capability void *c7, __capability void *c8) {

	if (op == CHERIJNI_METHOD_INIT) {

		struct cheri_object system_object;
		system_object.co_codecap = c1;
		system_object.co_datacap = c2;

		cheri_system_setup(system_object);
		cherijni_obj_system = system_object;
		cherijni_obj_init();
		cherijni_libc_init();

		return CHERI_SUCCESS;

	} else if (op == CHERIJNI_METHOD_LOOKUP) {

		/*
		 * Find method by its name stored in $c5
		 */

		char *method_name = cherijni_extractHostString(c1);
		void *method_ptr = cherijni_methodLookup(method_name);
		free(method_name);
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
//		char *signature = cherijni_extractHostString(c1);
		JNIEnv *env = cherijni_getJNIEnv();
		cherijni_jobject_clearLocal();

//		if (ranTests == JNI_FALSE) {
//			ranTests = JNI_TRUE;
//			cherijni_runTests(env);
//		}

//		register_t args_prim[] = { a2, a3, a4, a5, a6, a7 };
//		__capability void *args_objs[] = { c3, c4, c5, c6, c7, c8 };
//		size_t args_prim_ready = 0, args_objs_ready = 0;
//
//		jobject args_this = cherijni_jobject_store(c2, JNI_FALSE);
//		register_t args_ready[6];
//		jobject args_ready_obj[6];
//		scanSignature(signature,
//			/* single primitives */ { args_ready[args_prim_ready] = args_prim[args_prim_ready]; args_prim_ready++; },
//			/* double primitives */ { args_ready[args_prim_ready] = args_prim[args_prim_ready]; args_prim_ready++; },
//			/* objects           */ { args_ready_obj[args_objs_ready] = cherijni_jobject_store(args_objs[args_objs_ready], JNI_FALSE); args_objs_ready++; },
//			/* return values     */ { }, { }, { }, { });
//		free(signature);

		if (entry->type == FNTYPE_OBJECT) {
			__capability void *result_cap = ((fn_jni_obj) entry->func)(env, c2, a2, a3, a4, a5, a6, a7, c3, c4, c5, c6, c7, c8);
			cheri_setreg(3, result_cap);
			return CHERI_SUCCESS;
		} else {
			register_t result = ((fn_jni) entry->func)(env, c2, a2, a3, a4, a5, a6, a7, c3, c4, c5, c6, c7, c8);
			return result;
		}

	} else
		return CHERI_FAIL;
}
