#include "guest.h"

#define DEFAULT_OBJ_COUNT    2048

#define STORAGE_DEF(NAME)                                                                       \
	cherijni_objtype_##NAME *storage_##NAME = NULL;                                             \
	size_t                   storage_##NAME##_length = 0;                                       \
	                                                                                            \
	cherijni_objtype_##NAME *cherijni_obj_##NAME##_find(__capability void *cap) {               \
		size_t i;                                                                               \
		for (i = 0; i < storage_##NAME##_length; i++) {                                         \
			__capability void *cap_slot = storage_##NAME[i].cap;                                \
			if (cap == cap_slot)                                                                \
				return &storage_##NAME[i];                                                      \
		}                                                                                       \
		return NULL;                                                                            \
	}                                                                                           \
	                                                                                            \
	cherijni_objtype_##NAME *cherijni_obj_##NAME##_emptyslot() {                                \
		size_t i;                                                                               \
		for (i = 0; i < storage_##NAME##_length; i++) {                                         \
			__capability void *cap_slot = storage_##NAME[i].cap;                                \
			if (!cheri_gettag(cap_slot))                                                        \
				return &storage_##NAME[i];                                                      \
		}                                                                                       \
		printf("[SANDBOX ERROR: storage for %s is full!]\n", #NAME);                            \
		return NULL;                                                                            \
	}

#define STORAGE_INIT(NAME)                                                           \
	{                                                                                \
		storage_##NAME = calloc(DEFAULT_OBJ_COUNT, sizeof(cherijni_objtype_##NAME)); \
		if (storage_##NAME)                                                          \
			storage_##NAME##_length = DEFAULT_OBJ_COUNT;                             \
		else                                                                         \
			printf("[SANDBOX ERROR: cannot allocate storage for type %s]\n", #NAME); \
	}

STORAGE_DEF(jobject)
STORAGE_DEF(jfieldID)
STORAGE_DEF(jmethodID)
STORAGE_DEF(fd)
STORAGE_DEF(pFILE)

void cherijni_obj_init() {
	STORAGE_INIT(jobject);
	STORAGE_INIT(jfieldID);
	STORAGE_INIT(jmethodID);
	STORAGE_INIT(fd);
	STORAGE_INIT(pFILE);
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
		return NULL;                                                           \
	                                                                           \
	newslot->cap = cobj;


jobject cherijni_jobject_store(__capability void *cobj) {
	STORAGE_STORE_COMMON(jobject)
	return newslot;
}

jfieldID cherijni_jfieldID_store(__capability void *cobj) {
	STORAGE_STORE_COMMON(jfieldID)
	return newslot;
}

jmethodID cherijni_jmethodID_store(__capability void *cobj, const char *sig) {
	STORAGE_STORE_COMMON(jmethodID)
	newslot->sig = strdup(sig);
	return newslot;
}

int cherijni_fd_store(__capability void *cobj, int fd) {
	STORAGE_STORE_COMMON(fd)
	newslot->fd = fd;
	return fd;
}

__capability void *cherijni_fd_load(int fd) {
	size_t i;
	for (i = 0; i < storage_fd_length; i++)
		if (storage_fd[i].fd == fd)
			return storage_fd[i].cap;
	return CNULL;
}

void cherijni_fd_delete(int fd) {
	size_t i;
	for (i = 0; i < storage_fd_length; i++)
		if (storage_fd[i].fd == fd) {
			storage_fd[i].cap = cheri_zerocap();
			return;
		}
}

pFILE cherijni_pFILE_store(__capability void *cobj, short fileno) {
	STORAGE_STORE_COMMON(pFILE)
	newslot->fileno = fileno;
	return newslot;
}
