#include "guest.h"

#define DEFAULT_OBJ_COUNT    128

typedef struct cherijni_obj_storage {
	__capability void *caps[DEFAULT_OBJ_COUNT];
	size_t used_slots;
} ObjectStorage;

void cherijni_obj_init(struct _JNINativeInterface *env) {
	if (env->cherijni_objStorage == NULL) {
		ObjectStorage *store = malloc(sizeof(ObjectStorage));
		memset(store, 0, sizeof(ObjectStorage));
		env->cherijni_objStorage = store;
	}
}

#define CAP_BYTE(cap, i)    ( ((unsigned char*)(&cap))[i] )
#define CAP_EQUALS(c1, c2)	( (CAP_BYTE(c1, 0) == CAP_BYTE(c2, 0)) && \
                              (CAP_BYTE(c1, 1) == CAP_BYTE(c2, 1)) && \
                              (CAP_BYTE(c1, 2) == CAP_BYTE(c2, 2)) && \
                              (CAP_BYTE(c1, 3) == CAP_BYTE(c2, 3)) )

jobject cherijni_obj_storecap(JNIEnv *env, __capability void *cobj) {
	size_t i, j;

	// don't store NULLs
	if (!cheri_gettag(cobj) || (cobj == CNULL))
		return NULL;

	ObjectStorage *store = (ObjectStorage*) (*env)->cherijni_objStorage;

	// check if already stored
	for (i = 0; i < store->used_slots; i++)
		if (store->caps[i] == cobj)
			return &(store->caps[i]);

	// create a new slot;
	store->caps[store->used_slots] = cobj;
	return (jobject) &(store->caps[store->used_slots++]);
}

__capability void *cherijni_obj_loadcap(JNIEnv *env, jobject obj) {
	if (obj == NULL)
		return cheri_zerocap();
	else {
		__capability void *cap = *obj;
		return cap;
	}
}
