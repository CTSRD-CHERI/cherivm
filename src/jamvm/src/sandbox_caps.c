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

static __capability void *cherijni_seal   (pObject object, __capability void *sealcap);
static pObject            cherijni_unseal (__capability void *objcap, __capability void *sealcap);

static uintptr_t          type_Object;
static __capability void *sealcap_Object;

__capability void *cherijni_sealObject(pObject object) {
	return cherijni_seal(object, sealcap_Object);
}

pObject cherijni_unsealObject(__capability void *objcap) {
	return cherijni_unseal(objcap, sealcap_Object);
}

static uintptr_t          type_Context;
static __capability void *sealcap_Context;

__capability void *cherijni_sealContext(pClass context) {
	return cherijni_seal(context, sealcap_Context);
}

pClass cherijni_unsealContext(__capability void *objcap) {
	return (pClass) cherijni_unseal(objcap, sealcap_Context);
}

void cherijni_initCapabilities() {
	sealcap_Context = cheri_ptrtype(&type_Context, sizeof(uintptr_t), 0); // sets PERMIT_SEAL
	sealcap_Object = cheri_ptrtype(&type_Object, sizeof(uintptr_t), 0); // sets PERMIT_SEAL
}

static __capability void *cherijni_seal(pObject object, __capability void *sealcap) {
	if (object == NULL)
		return cheri_zerocap();

	__capability void *datacap_Object;
	datacap_Object = cheri_ptrperm(object, sizeof(Object),
	                                   CHERI_PERM_LOAD | CHERI_PERM_STORE |
	                                   CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP);
	datacap_Object = cheri_sealdata(datacap_Object, sealcap);

	return datacap_Object;
}

static pObject cherijni_unseal(__capability void *objcap, __capability void *sealcap) {
	// is it a valid capability?
	if (!cheri_gettag(objcap))
		return NULL;

	// is it sealed?
	if (cheri_getunsealed(objcap)) {
		jam_printf("Warning: provided cap cannot be unsealed (not sealed)\n");
		return NULL;
	}

	// is it of the correct type?
	if (cheri_gettype(objcap) != cheri_gettype(sealcap)) {
		jam_printf("Warning: provided cap cannot be unsealed (wrong type)\n");
		return NULL;
	}

	// unseal, convert to a pointer, return
	return (pObject) cheri_unseal(objcap, sealcap);
}

#endif
