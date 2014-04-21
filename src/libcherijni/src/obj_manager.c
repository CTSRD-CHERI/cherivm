#include "guest.h"

#define DEFAULT_OBJ_COUNT    128

static size_t obj_storage_length;
static size_t obj_storage_storeAt;
static __capability void **obj_storage = NULL;

void cherijni_obj_init() {
	if (obj_storage == NULL) {
		obj_storage_length = DEFAULT_OBJ_COUNT;
		obj_storage_storeAt = 0;
		obj_storage = malloc(obj_storage_length * sizeof(__capability void*));
		if (obj_storage == NULL) {
			printf("Error: cannot allocate object capability storage");
			abort();
		}
	}
}

#define CAP_BYTE(cap, i)    ( ((unsigned char*)(&cap))[i] )
#define CAP_EQUALS(c1, c2)	( (CAP_BYTE(c1, 0) == CAP_BYTE(c2, 0)) && \
                              (CAP_BYTE(c1, 1) == CAP_BYTE(c2, 1)) && \
                              (CAP_BYTE(c1, 2) == CAP_BYTE(c2, 2)) && \
                              (CAP_BYTE(c1, 3) == CAP_BYTE(c2, 3)) )

jobject cherijni_obj_storecap(__capability void *cobj) {
	size_t i, j;

	// don't store NULLs
	if (!cheri_gettag(cobj))
		return NULL;

//	printf("Finding the same cap\n");
//	// try to lookup the same capability
//	for (i = 0; i < obj_storage_length; i++) {
//		__capability void *cstored = obj_storage[i];
//		if (CAP_EQUALS(cobj, cstored))
//			return &obj_storage[i];
//	}

	// try to find a place to store it in
	for (i = 0; i < obj_storage_length; i++) {
		j = obj_storage_storeAt;

		obj_storage_storeAt++;
		if (obj_storage_length == obj_storage_storeAt)
			obj_storage_storeAt = 0;

		if (!cheri_gettag(obj_storage[j])) {
			obj_storage[j] = cobj;
			return &obj_storage[j];
		}
	}

	// TODO: resize storage

	return NULL;
}

__capability void *cherijni_obj_loadcap(jobject obj) {
	if (obj == NULL)
		return cheri_zerocap();
	else
		return *((__capability void**) obj);
}
