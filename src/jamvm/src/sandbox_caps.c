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

static uintptr_t type_JNIContext, type_JNIObject;
static __capability void *sealcap_JNIContext, *sealcap_JNIObject;

void cherijni_initCapabilities() {
	sealcap_JNIContext = cheri_ptrtype(&type_JNIContext, sizeof(uintptr_t), 0); // sets PERMIT_SEAL
	sealcap_JNIObject = cheri_ptrtype(&type_JNIObject, sizeof(uintptr_t), 0); // sets PERMIT_SEAL
}

__capability void *cherijni_sealJNIContext(pClass context) {
	jam_printf("[CHERIJNI: sealing context %s]\n", context ? CLASS_CB(context)->name : "NULL");

	if (context == NULL)
		return cheri_zerocap();

	__capability void *datacap_JNIContext;
	datacap_JNIContext = cheri_ptrperm(context, sizeof(Class),
	                                   CHERI_PERM_LOAD | CHERI_PERM_STORE |
	                                   CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP);
	datacap_JNIContext = cheri_sealdata(datacap_JNIContext, sealcap_JNIContext);

	return datacap_JNIContext;
}

pClass cherijni_unsealJNIContext(__capability void *cap) {

}

__capability void *cherijni_sealObject(pObject object) {
	if (object == NULL)
		return cheri_zerocap();

	__capability void *datacap_JNIObject;
	datacap_JNIObject = cheri_ptrperm(object, sizeof(Object),
	                                   CHERI_PERM_LOAD | CHERI_PERM_STORE |
	                                   CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP);
	datacap_JNIObject = cheri_sealdata(datacap_JNIObject, sealcap_JNIObject);

	return datacap_JNIObject;
}

#endif
