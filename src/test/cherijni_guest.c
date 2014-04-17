#include <sys/types.h>
#include <sys/stat.h>

#include <machine/cheri.h>
#include <machine/cheric.h>

#include <cheri/cheri_fd.h>
#include <cheri/cheri_invoke.h>
#include <cheri/cheri_memcpy.h>
#include <cheri/cheri_system.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

#ifndef __capability
#define __capability
#endif
#define METHOD_LOOKUP               0
#define METHOD_ONLOAD               1
#define METHOD_ONUNLOAD             2
#define METHOD_RUN                  3

typedef struct method_entry {
	char *name;
	void *func;
} methodEntry;

extern JavaVM *cherijni_getJavaVM();
extern JNIEnv *cherijni_getJNIEnv();

extern JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved);
extern JNIEXPORT jstring JNICALL Java_Prompt_getLine(JNIEnv *env, jobject self, jstring prompt);
extern JNIEXPORT void JNICALL Java_gnu_java_nio_VMChannel_initIDs(JNIEnv *env, jclass clazz);

static methodEntry methodList[] = {
	{"JNI_OnLoad", &JNI_OnLoad },
	{"Java_Prompt_getLine", &Java_Prompt_getLine },
	{"Java_gnu_java_nio_VMChannel_initIDs", &Java_gnu_java_nio_VMChannel_initIDs },
	{NULL, NULL}
};

static char* cherijni_extractHostString(__capability char* str_cap) {
	unsigned int i;
	size_t str_len = cheri_getlen(str_cap);
	char *buf = malloc(sizeof(char) * str_len);
	for (i = 0; i < str_len; i++)
		buf[i] = str_cap[i];
	return buf;
}

static void* cherijni_methodLookup(char *name) {
	methodEntry* entry = methodList;
	while (entry->name) {
		if (!strcmp(name, entry->name))
			return entry->func;
		entry++;
	}
	return NULL;
}

register_t cherijni_invoke(u_int op,
                           register_t a1, register_t a2, register_t a3,
                           register_t a4, register_t a5, register_t a6,
                           register_t a7, struct cheri_object system_object,
                           __capability void *c5, __capability void *c6,
                           __capability void *c7, __capability void *c8,
                           __capability void *c9, __capability void *c10) {

	cheri_system_setup(system_object);

	if (op == METHOD_LOOKUP) {

		/*
		 * Find method by its name stored in $c5
		 */

		char *method_name = cherijni_extractHostString(c5);
		void *method_ptr = cherijni_methodLookup(method_name);
		return (register_t) method_ptr;

	} else if (op == METHOD_ONLOAD) {

		/*
		 * Lookup and run (if found) JNI_OnLoad.
		 * Pointer to it is provided in $a1
		 */

		void* method_ptr = (void*) a1;
		jint result = (*(jint (*)(JavaVM*, void*)) method_ptr)(cherijni_getJavaVM(), NULL);
		return (register_t) result;

	} else if (op == METHOD_ONUNLOAD) {
		return (-1);
	} else if (op == METHOD_RUN) {

		/*
		 * Run arbitrary method with JNI call convention.
		 * Pointer provided in $a1
		 */

		void *method_ptr = (void*) a1;

	} else
		return (-1);
}
