#include "guest.h"

#define DEFAULT_OBJ_COUNT    128

#define STORAGE_DEF(NAME)                                                                       \
	cherijni_objtype_##NAME *storage_##NAME = NULL;                                             \
	size_t                   storage_##NAME##_length = 0;                                       \
	                                                                                            \
	cherijni_objtype_##NAME *cherijni_obj_##NAME##_find(__capability void *cap) {               \
		size_t i;                                                                               \
		for (i = 0; i < storage_##NAME##_length; i++) {                                         \
			__capability void *cap_slot = *((__capability void**) (&storage_##NAME[i]));        \
			if (cap == cap_slot)                                                                \
				return &storage_##NAME[i];                                                      \
		}                                                                                       \
		return NULL;                                                                            \
	}                                                                                           \
	                                                                                            \
	cherijni_objtype_##NAME *cherijni_obj_##NAME##_emptyslot() {                                \
		size_t i;                                                                               \
		for (i = 0; i < storage_##NAME##_length; i++) {                                         \
			__capability void *cap_slot = *((__capability void**) (&storage_##NAME[i]));        \
			if (!cheri_gettag(cap_slot))                                                        \
				return &storage_##NAME[i];                                                      \
		}                                                                                       \
		printf("[SANDBOX ERROR: storage for %s is full!]\n", #NAME);                            \
		return NULL;                                                                            \
	}

#define STORAGE_INIT(NAME)                                                           \
	{                                                                                \
		storage_##NAME = calloc(DEFAULT_OBJ_COUNT, sizeof(cherijni_objtype_##NAME)); \
		if (storage_##NAME) {                                                        \
			storage_##NAME##_length = DEFAULT_OBJ_COUNT;                             \
			printf("[SANDBOX initialized %s @ %p, len=%d]\n", #NAME, storage_##NAME, storage_##NAME##_length); \
		} else                                                                       \
			printf("[SANDBOX ERROR: cannot allocate storage for type %s]\n", #NAME); \
	}

STORAGE_DEF(jobject)
STORAGE_DEF(jfieldID)
STORAGE_DEF(jmethodID)
STORAGE_DEF(pFILE)

jboolean ranInit = JNI_FALSE;

void cherijni_obj_init() {
	if (ranInit == JNI_FALSE) {
		ranInit = JNI_TRUE;
		STORAGE_INIT(jobject);
		STORAGE_INIT(jfieldID);
		STORAGE_INIT(jmethodID);
		STORAGE_INIT(pFILE);
	}
}

#define STORAGE_STORE_COMMON(NAME)                                             \
	if (!cheri_gettag(cobj) || cobj == CNULL)                                  \
		return NULL;                                                           \
                                                                               \
	cherijni_objtype_##NAME *existing = cherijni_obj_##NAME##_find(cobj);      \
	if (existing)                                                              \
		return existing;                                                       \
                                                                               \
	cherijni_objtype_##NAME *newslot = cherijni_obj_##NAME##_emptyslot();      \
	if (!newslot)                                                              \
		return NULL;

jobject cherijni_jobject_store(__capability void *cobj) {
	STORAGE_STORE_COMMON(jobject)
	*newslot = cobj;
	return newslot;
}

jfieldID cherijni_jfieldID_store(__capability void *cobj) {
	STORAGE_STORE_COMMON(jfieldID)
	*newslot = cobj;
	return newslot;
}

jmethodID cherijni_jmethodID_store(__capability void *cobj, const char *sig) {
	STORAGE_STORE_COMMON(jmethodID)
	newslot->cap = cobj;
	newslot->sig = strdup(sig);
	return newslot;
}

pFILE cherijni_pFILE_store(__capability void *cobj) {
	STORAGE_STORE_COMMON(pFILE)
	newslot->cap = cobj;
	newslot->fileno = (((uintptr_t) newslot) - ((uintptr_t) storage_pFILE)) / sizeof(cherijni_objtype_pFILE);
	return newslot;
}
