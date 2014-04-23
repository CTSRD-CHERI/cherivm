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

static __capability void *cherijni_seal   (void *data, __capability void *sealcap);
static void              *cherijni_unseal (__capability void *datacap, __capability void *sealcap);

#define DEFINE_SEAL(TYPE, NAME)                                     \
        static uintptr_t          type_##NAME;                      \
        static __capability void *sealcap_##NAME;                   \
                                                                    \
        __capability void *cherijni_seal##NAME (TYPE data) {        \
        	return cherijni_seal(data, sealcap_##NAME);             \
        }                                                           \
                                                                    \
        TYPE cherijni_unseal##NAME (__capability void *datacap) {   \
        	return (TYPE) cherijni_unseal(datacap, sealcap_##NAME); \
        }
#define INIT_SEAL(NAME)         \
        sealcap_##NAME = cheri_ptrtype(&type_##NAME, sizeof(uintptr_t), 0); // sets PERMIT_SEAL

DEFINE_SEAL(pObject, JavaObject)
DEFINE_SEAL(pClass, Context)

void cherijni_initCapabilities() {
	INIT_SEAL(JavaObject);
	INIT_SEAL(Context);
}

static __capability void *cherijni_seal(void *object, __capability void *sealcap) {
	if (object == NULL)
		return cheri_zerocap();

	__capability void *datacap;
	datacap = cheri_ptrperm(object, sizeof(uintptr_t),
	                                   CHERI_PERM_LOAD | CHERI_PERM_STORE |
	                                   CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP);
	datacap = cheri_sealdata(datacap, sealcap);

	return datacap;
}

static void *cherijni_unseal(__capability void *objcap, __capability void *sealcap) {
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
	return (void*) cheri_unseal(objcap, sealcap);
}

#endif
