#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#include "jam.h"

#ifdef JNI_CHERI

#include "sandbox.h"
#include "sandbox_internal.h"
#include "sandbox_shared.h"

/*
 * Each capability must have its own type,
 * otherwise untrusted code could pass an object
 * of different type to the trampoline.
 */

static uintptr_t type_JNIEnv;
static __capability void *sealcap_JNIEnv;

void cherijni_initCapabilities() {
	sealcap_JNIEnv = cheri_ptrtype(&type_JNIEnv, sizeof(uintptr_t), 0); // sets PERMIT_SEAL
}

__capability void *cherijni_sealJNIEnv(JNIEnv *env) {
	jam_printf("[CHERIJNI: sealing JNIEnv %p]\n", env);

	__capability void *datacap_JNIEnv;
	datacap_JNIEnv = cheri_ptrperm(env, sizeof(JNIEnv), CHERI_PERM_LOAD);
	datacap_JNIEnv = cheri_sealdata(datacap_JNIEnv, sealcap_JNIEnv);

	CHERI_CAP_PRINT(datacap_JNIEnv);

	return datacap_JNIEnv;
}

#endif
