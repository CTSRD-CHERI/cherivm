#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

#include "jam.h"
#include "symbol.h"
#include "alloc.h"
#include "jni-internal.h"
#include "thread.h"

#ifdef JNI_CHERI

#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>

#include <cheri/sandbox.h>
#include <cheri/cheri_class.h>
#include <cheri/cheri_enter.h>
#include <cheri/cheri_system.h>
#include <cheri/cheri_invoke.h>

#include "sandbox.h"
#include "sandbox_shared.h"

extern JNIEnv globalJNIEnv;
static JNIEnv *env = &globalJNIEnv;
static pClass class_String;
static pClass class_Buffer;

static VMLock cherijni_sandbox_lock;

static void lockSandbox() {
	Thread *self = threadSelf();
    if(!tryLockVMLock(cherijni_sandbox_lock, self)) {
        disableSuspend(self);
        lockVMLock(cherijni_sandbox_lock, self);
        enableSuspend(self);
    }
//    printf("[LOCK: Thread %p entering sandbox]\n", self);
}

static void unlockSandbox() {
	Thread *self = threadSelf();
//    printf("[LOCK: Thread %p leaving sandbox]\n", self);
    unlockVMLock(cherijni_sandbox_lock, self);
}

struct sandbox_object {
	struct sandbox_class	*sbo_sandbox_classp;
	void			*sbo_mem;
	register_t		 sbo_sandboxlen;
	register_t		 sbo_heapbase;
	register_t		 sbo_heaplen;
	struct cheri_object	 sbo_cheri_object;
	struct cheri_object	 sbo_cheri_system_object;
	struct sandbox_object_stat	*sbo_sandbox_object_statp;
};

struct cap_counter cap_counter_new() {
	struct cap_counter counter;
	counter.base = 1;
	counter.length = 1;
	return counter;
}

struct cap_counter cap_counter_fromcap(__capability void *cap) {
	struct cap_counter counter;
	counter.base = cheri_getbase(cap);
	counter.length = cheri_getlen(cap);
	return counter;
}

void cap_counter_inc(struct cap_counter *counter) {
	uintptr_t max_length = cheri_getlen(cheri_getdefault());
	if (counter->base + counter->length < max_length)
		counter->length++;
	else {
		counter->base++;
		counter->length = 1;
	}
}

int cap_counter_compare(struct cap_counter *c1, struct cap_counter *c2) {
	if (c1->base > c2->base)
		return 1;

	if (c1->base < c2->base)
		return -1;

	if (c1->length > c2->length)
		return 1;

	if (c1->length < c2->length)
		return -1;

	return 0;
}

typedef struct cherijni_reference {
	jobject jni_ref;
	uintptr_t counter;
} cherijni_ref;

typedef cherijni_ref *pRef;

#define IS_REVOKED(ref)    (ref == NULL || (ref->jni_ref != NULL && ref->counter == 0))
#define IS_VALID(ref)      (ref != NULL && ref->jni_ref != NULL && ref->counter != 0)
#define FREE_REF(ref)      (ref->jni_ref = NULL)
#define IS_FREE(ref)       (ref->jni_ref == NULL)

#define SANDBOX_REFS_SIZE       128

struct cherijni_sandbox {
	struct sandbox_class       *classp;
	struct sandbox_object      *objectp;
	uintptr_t                   mem_start;
	uintptr_t                   mem_end;
	cherijni_ref                refs[SANDBOX_REFS_SIZE];
	size_t                      refs_size;
};

static void scrubMemory(struct cherijni_sandbox *sandbox);

#define DEFINE_SEAL_VARS(NAME)                                \
        static uintptr_t          type_##NAME;                      \
        static __capability void *sealcap_##NAME;                   \

#define cap_seal_len(NAME, data, len)    cherijni_seal(data, len, sealcap_##NAME)
#define cap_seal(NAME, data)             cap_seal_len(NAME, data, sizeof(uintptr_t))
#define cap_unseal(TYPE, NAME, datacap)  ((TYPE) cherijni_unseal(datacap, sealcap_##NAME))
#define cap_is_type(NAME, datacap)       cherijni_isType(datacap, sealcap_##NAME)
#define INIT_SEAL(NAME) sealcap_##NAME = cheri_ptrtype(&type_##NAME, sizeof(uintptr_t), 0); // sets PERMIT_SEAL

DEFINE_SEAL_VARS(Reference)
DEFINE_SEAL_VARS(MethodID)
DEFINE_SEAL_VARS(FieldID)
DEFINE_SEAL_VARS(FILE)

#define RETURNTYPE_VOID   1
#define RETURNTYPE_SINGLE 2
#define RETURNTYPE_DOUBLE 3
#define RETURNTYPE_OBJECT 4

static inline __capability void *cherijni_seal_cap(__capability void *datacap, __capability void *sealcap) {
	return cheri_sealdata(datacap, sealcap);
}

static inline __capability void *cherijni_seal(void *data, register_t len, __capability void *sealcap) {
	if (data == NULL)
		return CNULL;
	else
		return cherijni_seal_cap(cheri_ptrperm(data, len, CHERI_PERM_LOAD), sealcap);
}

static inline int cherijni_isType(__capability void *datacap, __capability void *sealcap) {
	// is it a valid capability?
	if (!cheri_gettag(datacap))
		return FALSE;

	// is it sealed?
	if (cheri_getunsealed(datacap))
		return FALSE;

	// is it of the correct type?
	if (cheri_gettype(datacap) != cheri_gettype(sealcap))
		return FALSE;

	return TRUE;
}

static inline __capability void *cherijni_unseal(__capability void *objcap, __capability void *sealcap) {
	if (objcap == CNULL)
		return NULL;

	if (!cherijni_isType(objcap, sealcap)) {
		jam_printf("Warning: provided cap cannot be unsealed\n");
		return NULL;
	}

	// unseal, return
	return cheri_unseal(objcap, sealcap);
}

char *cherijni_libName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof(".cheri") + 1);

   sprintf(buff, "%s.cheri", name);
   return buff;
}

#define CInvoke_7_6(handle, op, a1, a2, a3, a4, a5, a6, a7, c3, c4, c5, c6, c7, c8, c9, c10)                 \
		sandbox_object_cinvoke (                                                                             \
				((struct cherijni_sandbox *) handle)->objectp, op,                                           \
	            (register_t) a1, (register_t) a2, (register_t) a3, (register_t) a4,                          \
                (register_t) a5, (register_t) a6, (register_t) a7,                                           \
	            c3, c4, c5, c6, c7, c8, c9, c10)
#define CInvoke_2_2(handle, op, a1, a2, c3, c4)  CInvoke_7_6(handle, op, a1, a2, 0, 0, 0, 0, 0, c3, c4, CNULL, CNULL, CNULL, CNULL, CNULL, CNULL)
#define CInvoke_0_2(handle, op, c3, c4)          CInvoke_2_2(handle, op, 0, 0, c3, c4)
#define CInvoke_1_1(handle, op, a1, c3)          CInvoke_2_2(handle, op, a1, 0, c3, CNULL)
#define CInvoke_1_0(handle, op, a1)              CInvoke_1_1(handle, op, a1, CNULL)
#define CInvoke_0_1(handle, op, c5)              CInvoke_1_1(handle, op, 0, c5)
#define CInvoke_0_0(handle, op)                  CInvoke_1_1(handle, op, 0, CNULL)

struct sandbox_list {
	struct cherijni_sandbox *handle;
	struct sandbox_list *next;
};

static struct sandbox_list *sandboxes = NULL;

void *cherijni_open(char *path) {
	struct cherijni_sandbox *sandbox = sysMalloc(sizeof(struct cherijni_sandbox));

	/* Initialize the sandbox class and object */
	if (sandbox_class_new(path, DEFAULT_SANDBOX_MEM, &sandbox->classp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	if (sandbox_object_new(sandbox->classp, &sandbox->objectp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	/* Store memory space limits */
	sandbox->mem_start = (uintptr_t) sandbox_object_getbase(sandbox->objectp);
	sandbox->mem_end = sandbox->mem_start + sandbox_class_getlength(sandbox->classp);

	/* Setup reference array */
	sandbox->refs_size = SANDBOX_REFS_SIZE;
	memset(sandbox->refs, 0, sandbox->refs_size * sizeof(cherijni_ref));

	/* Run init inside sandbox */
	struct cheri_object system = sandbox_object_getsystemobject(sandbox->objectp);
	lockSandbox();
	register_t result = CInvoke_0_2(sandbox, CHERIJNI_METHOD_INIT, system.co_codecap, system.co_datacap);
	unlockSandbox();
	if (result == CHERI_FAIL) {
		jam_printf("ERROR: sandbox failed to initialize (%s)\n", path);
		sysFree(sandbox);
		return NULL;
	}

	struct sandbox_list *new_entry = sysMalloc(sizeof(struct sandbox_list));
	new_entry->handle = sandbox;
	new_entry->next = sandboxes;
	sandboxes = new_entry;

	return sandbox;
}

void cherijni_scrubMemory() {
	struct sandbox_list *entry = sandboxes;
	while (entry != NULL) {
		scrubMemory(entry->handle);
		entry = entry->next;
	}
}

void cherijni_markValidRefs() {
	size_t ref_count, i;
	pRef ref_slot;

	struct sandbox_list *entry = sandboxes;
	while (entry != NULL) {
		ref_count = entry->handle->refs_size;
		ref_slot = entry->handle->refs;

		for (i = 0; i < ref_count; i++, ref_slot++) {
			if (IS_VALID(ref_slot)) {
				// printf("[CHERI: marked %s @ %p (%d)]\n", CLASS_CB(REF_TO_OBJ_WEAK_NULL_CHECK(ref_slot->jni_ref)->class)->name, ref_slot->jni_ref, REF_TYPE(ref_slot->jni_ref));
				markConservativeRoot(REF_TO_OBJ_WEAK_NULL_CHECK(ref_slot->jni_ref));
			}
		}

		entry = entry->next;
	}
}

// TODO: make sure only one thread ever enters the sandbox !!!
// TODO: check the return value? -1 *may* mean that a trap happened inside the sandbox (will be replaced with signals)

void *cherijni_lookup(void *handle, char *methodName) {
	lockSandbox();
	void *res = (void*) CInvoke_0_1(handle, CHERIJNI_METHOD_LOOKUP, cap_string(methodName));
	unlockSandbox();
	return res;
}

jint cherijni_callOnLoadUnload(void *handle, void *ptr, JavaVM *jvm, void *reserved) {
	lockSandbox();
	jint res = (jint) CInvoke_0_0(handle, CHERIJNI_METHOD_ONLOAD_ONUNLOAD);
	unlockSandbox();
	return res;
}

static inline __capability int checkSandboxCapability(__capability void *cap, size_t min_length, register_t perm_mask, int unsealed) {
	if (!cheri_gettag(cap)) {
		jam_printf("Warning: sandbox provided an invalid capability\n");
		return FALSE;
	}

	if (cheri_getunsealed(cap) != unsealed) {
		jam_printf("Warning: sandbox provided a capability with wrong unsealed tag (expected %d)\n", unsealed);
		return FALSE;
	}

	if (min_length > 0 && cheri_getlen(cap) < min_length) {
		jam_printf("Warning: sandbox provided a pointer capability which is too short\n");
		return FALSE;
	}

	register_t perm_cap = cheri_getperm(cap);
	if ((perm_cap & perm_mask) != perm_mask) {
		jam_printf("Warning: sandbox provided a pointer capability with insufficient permissions\n");
		return FALSE;
	}

	return TRUE;
}

static inline int checkSandboxCapability_r(__capability void *cap, size_t min_length, int unsealed) {
	return checkSandboxCapability(cap, min_length, CHERI_PERM_LOAD, unsealed);
}

static inline int checkSandboxCapability_w(__capability void *cap, size_t min_length, int unsealed) {
	return checkSandboxCapability(cap, min_length, CHERI_PERM_STORE, unsealed);
}

#define arg_cap(cap, len, perm, u)    (checkSandboxCapability_##perm(cap, len, u) ? cap : CNULL)
#define arg_ptr(cap, len, perm)       ((void*) arg_cap(cap, len, perm, TRUE))
#define arg_str(ptr, len, perm)       ((const char*) arg_ptr(ptr, len, perm)) // TODO: check it ends with a zero
#define arg_class(cap)                checkIsClass(arg_jniref(cap))
#define arg_file(cap)                 cap_unseal(pFILE, FILE, cap)
#define arg_mid(cap)                  cap_unseal(pMethodBlock, MethodID, cap)
#define return_mid(field)             cap_seal(MethodID, field)
#define return_fid(field)             cap_seal(FieldID, field)
#define return_file(file)             cap_seal(FILE, file)
#define return_str(str)               cap_string(str)

#define FD_COUNT (1 << 16)
static struct cap_counter FDs[FD_COUNT];

#define RETURN_FUNC(NAME, param, counter_cmd, ptr)                                                \
	static inline __capability void *return_##NAME(param) {                                       \
		struct cap_counter counter = counter_cmd;                                                 \
		__capability void *sealcap = cheri_ptrtype(ptr, sizeof(uintptr_t), 0);                    \
		return cherijni_seal((void*) counter.base, counter.length, sealcap);                      \
	}

static inline __capability void *return_fd(int fd) {
	if (fd < 0)
		return CNULL;

	struct cap_counter counter = FDs[fd];
	__capability void *sealcap = cheri_ptrtype(&FDs[fd], sizeof(uintptr_t), 0);
	return cherijni_seal((void*) counter.base, counter.length, sealcap);
}

static inline int arg_fd(__capability void *cap) {
	if (cap == CNULL || !checkSandboxCapability(cap, 0, 0, FALSE))
		return -1;

	uintptr_t type_size = sizeof(FDs[0]);

	uintptr_t type_start = (uintptr_t) FDs;
	uintptr_t type_end = type_start + type_size * FD_COUNT;

	uintptr_t type_cap = cheri_gettype(cap);
	uintptr_t type_cap_offset = type_cap - type_start;

	if (type_cap < type_start || type_cap >= type_end || (type_cap_offset % type_size) != 0) {
		jam_printf("Warning: sandbox provided an invalid file-descriptor capability (wrong type)");
		return -1;
	}

	int fd = type_cap_offset / type_size;
	struct cap_counter fd_counter = cap_counter_fromcap(cap);

	if (cap_counter_compare(&FDs[fd], &fd_counter) == 0)
		return fd;
	else {
		jam_printf("Warning: sandbox provided a revoked file descriptor (fd=%d, counter: %d|%d != %d|%d)\n", fd, fd_counter.base, fd_counter.length, FDs[fd].base, FDs[fd].length);
		return -1;
	}
}

static inline void revoke_fd(int fd) {
	cap_counter_inc(&FDs[fd]);
}

#define GET_SANDBOX_HANDLE(error_return)                                     \
		struct cherijni_sandbox *sandbox;                                    \
		Frame *frame = getExecEnv()->last_frame;                             \
		if (frame->mb != NULL && frame->mb->sandbox_handle != NULL)          \
			sandbox = (struct cherijni_sandbox *) frame->mb->sandbox_handle; \
		else {                                                               \
			jam_printf("[ERROR: cannot find sandbox handle]\n");             \
			return error_return;                                             \
		}

static inline __capability void *return_ref(pRef ref) {
	if (ref == NULL)
		return CNULL;
	else {
		ref->counter++;
		if (ref->counter == 0) {
			// buffer has overflown
			jam_printf("[ERROR: Reference counter overflow. Exitting...]\n");
			exitVM(1);
		}
		return cap_seal(Reference, ref);
	}
}

static inline __capability void *return_jniref(jobject jniref) {
	size_t i;
	pRef slot;
	GET_SANDBOX_HANDLE(CNULL)

	if (jniref == NULL)
		return return_ref(NULL);

	// check if the reference has a slot
	for (i = 0, slot = sandbox->refs; i < sandbox->refs_size; i++, slot++) {
		if (slot->jni_ref == jniref)
			return return_ref(slot);
	}

	int scrubbedMemory = FALSE;
	while (TRUE) {

		// reference doesn't exist, create it
		for (i = 0, slot = sandbox->refs; i < sandbox->refs_size; i++, slot++) {
			if (IS_FREE(slot)) {
				slot->jni_ref = jniref;
				slot->counter = 0;
				return return_ref(slot);
			}
		}

		if (scrubbedMemory) {
			jam_printf("[ERROR: Sandbox requested too many references. Exiting...]\n");
			exitVM(1);
		} else {
			scrubbedMemory = TRUE;
			scrubMemory(sandbox);
		}
	}

	return CNULL;
}

static inline int checkIsReference(uintptr_t offset_ref, struct cherijni_sandbox *sandbox) {
	uintptr_t offset_start, offset_end;
	offset_start = (uintptr_t) sandbox->refs;
	offset_end = offset_start + sandbox->refs_size * sizeof(cherijni_ref);
	return (offset_start <= offset_ref) &&
	       (offset_ref < offset_end) &&
	       ((offset_ref - offset_start) % sizeof(cherijni_ref) == 0);
}

static inline pRef arg_ref_allowRevoked(__capability void *cap, struct cherijni_sandbox *sandbox) {
	if (cap == CNULL)
		return NULL;

	if (!checkSandboxCapability(cap, 0, 0, FALSE)) {
		jam_printf("Warning: sandbox provided an invalid jobject capability\n");
		return NULL;
	}

	pRef ref = cap_unseal(pRef, Reference, cap);
	if (!checkIsReference((uintptr_t) ref, sandbox)) {
		jam_printf("Warning: sandbox provided a reference outside the reference table\n");
		return NULL;
	}

	return ref;
}

static inline pRef arg_ref(__capability void *cap) {
	GET_SANDBOX_HANDLE(NULL)

	pRef ref = arg_ref_allowRevoked(cap, sandbox);

	if (!IS_VALID(ref)) {
		jam_printf("Warning: sandbox provided a revoked reference\n");
		return NULL;
	}

	return ref;
}

static inline jobject arg_jniref(__capability void *cap) {
	pRef ref = arg_ref(cap);
	if (ref == NULL)
		return NULL;
	else
		return ref->jni_ref;
}

static inline void revoke_ref(pRef ref) {
	if (ref == NULL)
		return;

	if (ref->counter == 0) {
		jam_printf("[ERROR: Asked to revoke an already revoked reference]\n");
		return;
	}

//	printf("[REVOKE: ref %s @ %p | %lu => %lu]\n", CLASS_CB(REF_TO_OBJ_WEAK_NULL_CHECK(ref->jni_ref)->class)->name, ref->jni_ref, ref->counter, ref->counter - 1);
	ref->counter--;
}

static inline void revoke_jniref(jobject jniref) {
	size_t i;
	pRef slot;
	GET_SANDBOX_HANDLE()

	if (jniref == NULL)
		return;

	// check if the reference has a slot
	for (i = 0, slot = sandbox->refs; i < sandbox->refs_size; i++, slot++) {
		if (slot->jni_ref == jniref) {
			revoke_ref(slot);
			return;
		}
	}

	jam_printf("[ERROR: Asked to revoke non-existent JNI reference %p]\n", jniref);
	exitVM(1);
}

static void scrubMemory_region(uintptr_t start, uintptr_t end, struct cherijni_sandbox *sandbox) {
	__capability void *slot_cap, **slot_ptr;
	slot_ptr = (__capability void **) start;
	for (; (uintptr_t) slot_ptr < end; slot_ptr++) {
		slot_cap = *slot_ptr;
		if (cheri_gettag(slot_cap) && cap_is_type(Reference, slot_cap)) {
			pRef ref = arg_ref_allowRevoked(slot_cap, sandbox);
			if (IS_REVOKED(ref)) {
//				printf("[SCRUB: erasing revoked cap to %s @ %p]\n", CLASS_CB(REF_TO_OBJ_WEAK_NULL_CHECK(ref->jni_ref)->class)->name, ref->jni_ref);
				*slot_ptr = cheri_zerocap();
			}
		}
	}
}

#define	STACK_SIZE	(32*PAGE_SIZE)

static void scrubMemory(struct cherijni_sandbox *sandbox) {
	uintptr_t start_stack, start_heap, end_stack, end_heap;
	pRef ref_slot;
	size_t i;

	/* Compute memory intervals */
	end_stack = sandbox->mem_end;
	start_stack = end_stack - STACK_SIZE;

	start_heap = sandbox->mem_start + sandbox->objectp->sbo_heapbase;
	end_heap = start_heap + sandbox->objectp->sbo_heaplen;

	/* Scrub the memory */
	scrubMemory_region(start_stack, end_stack, sandbox);
	scrubMemory_region(start_heap, end_heap, sandbox);

	/* Walk through the revocation table and free revoked caps */
	for (i = 0, ref_slot = sandbox->refs; i < sandbox->refs_size; i++, ref_slot++)
		if (IS_REVOKED(ref_slot))
			ref_slot->jni_ref = NULL;
}

static inline void copyToSandbox(__capability char *dest, const char *src, size_t len) {
	size_t i;
	for (i = 0; i < len; i++)
		dest[i] = src[i];
}

static inline void copyFromSandbox(char *dest, __capability char *src, size_t len) {
	size_t i;
	for (i = 0; i < len; i++)
		dest[i] = src[i];
}

static inline pClass checkIsClass(jobject jniref) {
	pObject obj = REF_TO_OBJ_WEAK_NULL_CHECK(jniref);
	if (obj == NULL)
		return NULL;
	else if (IS_CLASS(obj))
		return (pClass) obj;
	else {
		jam_printf("Warning: expected a class object from sandbox\n");
		return NULL;
	}
}

#define checkIsValid(obj, NAME) ((obj != NULL) && isInstanceOf(REF_TO_OBJ_WEAK_NULL_CHECK(obj)->class, class_##NAME))

static inline int checkIsArray(jobject ref, char type) {
	pObject obj = REF_TO_OBJ_WEAK_NULL_CHECK(ref);
	if (obj == NULL)
		return FALSE;

	if (!IS_ARRAY(CLASS_CB(obj->class)))
		return FALSE;

	return type == 0 || CLASS_CB(obj->class)->name[1] == type;
}

#define FRAMETYPE_DUMMY 0
#define FRAMETYPE_JAVA  1
#define FRAMETYPE_JNI   2

static int getFrameType(Frame *frame) {
	if (frame->mb == NULL)
		return FRAMETYPE_DUMMY;
	else if (frame->mb->access_flags & ACC_NATIVE)
		return FRAMETYPE_JNI;
	else
		return FRAMETYPE_JAVA;
}

static void revokeLrefsInFrame(JNIFrame *frame) {
	pObject *lref;
//	printf("[REVOKE FRAME: lrefs=%p next_ref=%p len=%lu]\n", frame->lrefs, frame->next_ref, ((uintptr_t)frame->next_ref - (uintptr_t)frame->lrefs) / sizeof(jobject));
	for(lref = frame->lrefs; lref < frame->next_ref; lref++)
		revoke_jniref(*lref);
}

/*
 * XXX: HACKISH!!! Bypassing cheri_sandbox_cinvoke
 */
static __capability void *invoke_returnCap(void *handle, void *native_func, __capability void *cap_signature, __capability void *cap_this, register_t args_prim[], __capability void *args_cap[]) {
	cheri_invoke_cap func = (cheri_invoke_cap) cheri_invoke;
	struct cherijni_sandbox *sandbox = (struct cherijni_sandbox*) handle;
	lockSandbox();
	__capability void *res = (func)(
			sandbox->objectp->sbo_cheri_object,
			CHERIJNI_METHOD_RUN, (register_t) native_func,
			args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5],
            cap_signature, cap_this,
            args_cap[0], args_cap[1], args_cap[2], args_cap[3], args_cap[4], args_cap[5]);
	unlockSandbox();
	return res;
}

static register_t invoke_returnPrim(void *handle, void *native_func, __capability void *cap_signature, __capability void *cap_this, register_t args_prim[], __capability void *args_cap[]) {
	lockSandbox();
	register_t res = CInvoke_7_6(handle, CHERIJNI_METHOD_RUN, native_func,
	                   args_prim[0], args_prim[1], args_prim[2], args_prim[3], args_prim[4], args_prim[5],
	                   cap_signature, cap_this,
	                   args_cap[0], args_cap[1], args_cap[2], args_cap[3], args_cap[4], args_cap[5]);
	unlockSandbox();
	return res;
}

static void fillArguments(char *sig, uintptr_t *_ostack, register_t *_pPrimitiveArgs, __capability void **_pObjectArgs) {
	scanSignature(sig,
		/* single primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); },
		/* double primitives */ { *(_pPrimitiveArgs++) = *(_ostack++); _ostack++; },
		/* objects           */ { jobject newref = OBJ_TO_REF((jobject) *_ostack, LOCAL_REF);
		                          *(_pObjectArgs++) = return_jniref(newref);
		                          (*env)->NewLocalRef(env, newref);
		                          _ostack++; },
		/* return values     */ { }, { }, { }, { });
}

uintptr_t *cherijni_callMethod(pMethodBlock mb, pClass class, uintptr_t *ostack) {
	__capability void *cap_this;
	uintptr_t *_ostack = ostack;
	size_t cPrimitiveArgs = 0, cObjectArgs = 0, returnType;
	register_t args_prim [6] = {0, 0, 0, 0, 0, 0};
	__capability void *args_cap [6] = {CNULL, CNULL, CNULL, CNULL, CNULL, CNULL};

	/* Count the arguments */
	scanSignature(mb->type,
		/* single primitives */ { (cPrimitiveArgs++); },
		/* double primitives */ { (cPrimitiveArgs++); },
		/* objects           */ { (cObjectArgs++); },
		/* return values     */ { returnType = RETURNTYPE_VOID; },
		                        { returnType = RETURNTYPE_SINGLE; },
		                        { returnType = RETURNTYPE_DOUBLE; },
		                        { returnType = RETURNTYPE_OBJECT; });

	// TODO: check number of arguments

	/* Set JNI frame depth to zero */
	if ((*env)->PushLocalFrame(env, 16) == JNI_ERR) {
		jam_printf("[ERROR: Could not a new frame for CheriJNI call. Exiting...]\n");
		exitVM(1);
	}
	getExecEnv()->last_frame->depth = 0;

	/* Prepare arguments */
	if (class == NULL) { cap_this = return_jniref(OBJ_TO_REF((jobject) *_ostack, LOCAL_REF)); _ostack++; }
	else                 cap_this = return_jniref(OBJ_TO_REF(class, LOCAL_REF));
	fillArguments(mb->type, _ostack, args_prim, args_cap);

	/* Invoke JNI method */

	struct cherijni_sandbox *handle = (struct cherijni_sandbox *) mb->sandbox_handle;
	void *native_func = mb->code;
	if (returnType == RETURNTYPE_OBJECT) {
		__capability void *cap_result = invoke_returnCap(handle, native_func, cap_string(mb->type), cap_this, args_prim, args_cap);
		pObject result_obj = REF_TO_OBJ_WEAK_NULL_CHECK(arg_jniref(cap_result));
		*(ostack++) = (uintptr_t) result_obj;
	} else {
		register_t result = invoke_returnPrim(handle, native_func, cap_string(mb->type), cap_this, args_prim, args_cap);
		if (returnType == RETURNTYPE_SINGLE)
			*(ostack++) = result;
		else if (returnType == RETURNTYPE_DOUBLE) {
			*(ostack++) = result;
			ostack++;
		}
	}

	while(TRUE) {
		JNIFrame *frame = (JNIFrame*) getExecEnv()->last_frame;
		revokeLrefsInFrame(frame);
		if (frame->depth == 0)
			break;
		else
			(*env)->PopLocalFrame(env, NULL);
	}

	return ostack;
}

#define JNI_FUNCTION_PRIM(NAME)     static register_t          JNI_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4, __capability void *c5)
#define JNI_FUNCTION_CAP(NAME)      static __capability void  *JNI_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4, __capability void *c5)
#define LIBC_FUNCTION_PRIM(NAME)    static register_t          LIBC_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4, __capability void *c5)
#define LIBC_FUNCTION_CAP(NAME)     static __capability void * LIBC_##NAME (register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4, __capability void *c5)

#define CALL_JNI_PRIM(NAME)         { return JNI_##NAME (a1, a2, a3, a4, a5, a6, a7, c1, c2, c3, c4, c5); }
#define CALL_JNI_CAP(NAME)          { __capability void *result = JNI_##NAME (a1, a2, a3, a4, a5, a6, a7, c1, c2, c3, c4, c5); cheri_setreg(3, result); return CHERI_SUCCESS; }
#define CALL_LIBC_PRIM(NAME)        { return LIBC_##NAME (a1, a2, a3, a4, a5, a6, a7, c1, c2, c3, c4, c5); }
#define CALL_LIBC_CAP(NAME)         { __capability void *result = LIBC_##NAME (a1, a2, a3, a4, a5, a6, a7, c1, c2, c3, c4, c5); cheri_setreg(3, result); return CHERI_SUCCESS; }

JNI_FUNCTION_PRIM(GetVersion) {
	return (*env)->GetVersion(env);
}

JNI_FUNCTION_CAP(FindClass) {
	const char *className = arg_str(c1, 1, r);
	if (className == NULL)
		return CNULL;

	return return_jniref((*env)->FindClass(env, className));
}

JNI_FUNCTION_PRIM(ThrowNew) {
	pClass clazz = arg_class(c1);
	const char *msg = arg_str(c2, 1, r);
	if (clazz == NULL)
		return CHERI_FAIL;

	return (*env)->ThrowNew(env, clazz, msg);
}

JNI_FUNCTION_CAP(ExceptionOccurred) {
	return return_jniref((*env)->ExceptionOccurred(env));
}

JNI_FUNCTION_PRIM(ExceptionDescribe) {
	(*env)->ExceptionDescribe(env);
	return CHERI_SUCCESS;
}

JNI_FUNCTION_PRIM(ExceptionClear) {
	(*env)->ExceptionClear(env);
	return CHERI_SUCCESS;
}

JNI_FUNCTION_PRIM(PushLocalFrame) {
	jint capacity = a1;
	jint result = (*env)->PushLocalFrame(env, capacity);
	getExecEnv()->last_frame->depth++;
    return result;
}

JNI_FUNCTION_CAP(PopLocalFrame) {
	JNIFrame *frame = (JNIFrame*) getExecEnv()->last_frame;
	if (frame->depth == 0) {
		jam_printf("Warning: sandbox attempted to pop the root JNI frame\n");
		return CNULL;
	}

	jobject ref = arg_jniref(c1);
	revokeLrefsInFrame(frame);
	jobject result = (*env)->PopLocalFrame(env, ref);
	// TODO: should it increase the counter?
	return return_jniref(result);
}

JNI_FUNCTION_CAP(NewGlobalRef) {
	jobject ref = arg_jniref(c1);
	if (ref == NULL)
		return CNULL;

	jobject result = (*env)->NewGlobalRef(env, ref);
	return return_jniref(result);
}

JNI_FUNCTION_PRIM(DeleteGlobalRef) {
	pRef globalRef = arg_ref(c1);
	if (!IS_VALID(globalRef) || REF_TYPE(globalRef->jni_ref) != GLOBAL_REF)
		return CHERI_FAIL;

	/*
	* Attempt to delete a global ref from the table.
	* If successful, decrement the reference counter.
	*/
	if (delJNIGref(REF_TO_OBJ(globalRef->jni_ref), GLOBAL_REF)) {
		revoke_ref(globalRef);
		return CHERI_SUCCESS;
	} else {
		jam_printf("Warning: sandbox requested to delete a global ref not present in its current frame\n");
		return CHERI_FAIL;
	}
}


JNI_FUNCTION_PRIM(DeleteLocalRef) {
	pRef localRef = arg_ref(c1);
	if (localRef == NULL || localRef->jni_ref == NULL || REF_TYPE(localRef->jni_ref) != LOCAL_REF)
		return CHERI_FAIL;

	/*
	 * Attempt to delete a local ref from the current frame.
	 * If successful, decrement the reference counter.
	 */
	if (delJNILref(localRef->jni_ref)) {
		revoke_ref(localRef);
		return CHERI_SUCCESS;
	} else {
		jam_printf("Warning: sandbox requested to delete a local ref not present in its current frame\n");
		return CHERI_FAIL;
	}
}

JNI_FUNCTION_PRIM(IsSameObject) {
	jobject ref1 = arg_jniref(c1);
	jobject ref2 = arg_jniref(c2);

	return (*env)->IsSameObject(env, ref1, ref2);
}

JNI_FUNCTION_CAP(NewLocalRef) {
	jobject ref = arg_jniref(c1);
	if (ref == NULL)
		return CNULL;

	jobject result = (*env)->NewLocalRef(env, ref);
	return return_jniref(result);
}

JNI_FUNCTION_PRIM(IsInstanceOf) {
	jobject obj = arg_jniref(c1);
	pClass clazz = arg_class(c2);
	if (clazz == NULL)
		return CHERI_FAIL;

	return (register_t) (*env)->IsInstanceOf(env, obj, clazz);
}

JNI_FUNCTION_CAP(GetMethodID) {
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CNULL;

	pMethodBlock result = (*env)->GetMethodID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkMethodAccess(result, context)) {
		jam_printf("Warning: sandbox requested a method outside its execution context\n");
		result = NULL;
	}
#endif
	return return_mid(result);
}

static inline jvalue *prepareJniArguments(pMethodBlock mb, register_t a1, register_t a2,
		                                  register_t a3, register_t a4, register_t a5,
		                                  register_t a6, register_t a7,
		                                  __capability void *c3, __capability void *c4, __capability void *c5) {
	int arg_count = 0;
	scanSignature(mb->type,
			{ arg_count++; }, { arg_count++; }, { arg_count++; },
			{ }, { }, { }, { });
	if (arg_count == 0)
		return NULL;

	jvalue *args = (jvalue*) sysMalloc(sizeof(jvalue) * arg_count);
	register_t args_prim[] = { a1, a2, a3, a4, a5, a6, a7 };
	__capability void *args_cap[] = { c3, c4, c5 };
	int args_ready = 0, args_used_prim = 0, args_used_cap = 0;
	scanSignature(mb->type,
	/* single primitives */ { args[args_ready++].i = args_prim[args_used_prim++]; },
	/* double primitives */ { args[args_ready++].j = args_prim[args_used_prim++]; },
	/* objects           */ { args[args_ready++].l = arg_jniref(args_cap[args_used_cap]); args_used_cap++; },
	/* return values     */ { }, { }, { }, { });

	return args;
}

#define VIRTUAL_METHOD_COMMON(failReturn)                                            \
	jobject obj = arg_jniref(c1);                                                    \
	pMethodBlock mb = arg_mid(c2);                                                   \
	if (obj == NULL || mb == NULL)                                                   \
		return failReturn;                                                           \
	jvalue *args = prepareJniArguments(mb, a1, a2, a3, a4, a5, a6, a7, c3, c4, c5);

#define VIRTUAL_METHOD_PRIM(TYPE, jtype)                                \
	JNI_FUNCTION_PRIM(Call##TYPE##Method) {                             \
		VIRTUAL_METHOD_COMMON(CHERI_FAIL)                               \
		jtype result = (jtype) (*env)->Call##TYPE##MethodA(env, obj, mb, args); \
		if (args) sysFree(args); \
		return result; \
	}

#define CALL_METHOD(access)             \
access##_METHOD_PRIM(Boolean, jboolean) \
access##_METHOD_PRIM(Byte, jbyte)       \
access##_METHOD_PRIM(Char, jchar)       \
access##_METHOD_PRIM(Short, jshort)     \
access##_METHOD_PRIM(Int, jint)         \
access##_METHOD_PRIM(Long, jlong)       \
access##_METHOD_PRIM(Float, jfloat)     \
access##_METHOD_PRIM(Double, jdouble)

CALL_METHOD(VIRTUAL)

JNI_FUNCTION_CAP(CallObjectMethod) {
	VIRTUAL_METHOD_COMMON(CNULL)
	jobject result = (*env)->CallObjectMethodA(env, obj, mb, args);
	if (args) sysFree(args);
	return return_jniref(result);
}

JNI_FUNCTION_CAP(GetFieldID) {
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CNULL;

	pFieldBlock result = (*env)->GetFieldID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkFieldAccess(result, context)) {
		jam_printf("Warning: sandbox requested a field outside its execution context\n");
		result = CNULL;
	}
#endif
	return return_fid(result);
}

JNI_FUNCTION_CAP(GetStaticMethodID) {
	pClass clazz = arg_class(c1);
	const char *name = arg_str(c2, 1, r);
	const char *sig = arg_str(c3, 1, r);
	if (clazz == NULL || name == NULL || sig == NULL)
		return CNULL;

	pMethodBlock result = (*env)->GetStaticMethodID(env, clazz, name, sig);
#ifdef JNI_CHERI_STRICT
	if (!checkMethodAccess(result, context)) {
		jam_printf("Warning: sandbox requested a static method outside its execution context\n");
		result = CNULL;
	}
#endif
	return return_mid(result);
}

JNI_FUNCTION_CAP(NewStringUTF) {
	const char *bytes = arg_str(c1, 1, r);
	if (bytes == NULL)
		return CNULL;

	return return_jniref((*env)->NewStringUTF(env, bytes));
}

JNI_FUNCTION_PRIM(GetStringUTFLength) {
	jobject string = arg_jniref(c1);
	if (string == NULL)
		return CHERI_FAIL;

	if (!checkIsValid(string, String))
		return CHERI_FAIL;
	return (*env)->GetStringUTFLength(env, string);
}

JNI_FUNCTION_PRIM(GetStringUTFChars) {
	jobject string = arg_jniref(c1);
	if (!checkIsValid(string, String))
		return CHERI_FAIL;
	jsize str_length = (*env)->GetStringUTFLength(env, string);

	__capability char *sandbox_buffer = arg_cap(c2, str_length + 1, w, TRUE);
	if (sandbox_buffer == CNULL)
		return CHERI_FAIL;

	const char *host_buffer = (*env)->GetStringUTFChars(env, string, NULL);
	copyToSandbox(sandbox_buffer, host_buffer, str_length + 1);
	(*env)->ReleaseStringUTFChars(env, string, host_buffer);

	return CHERI_SUCCESS;
}

JNI_FUNCTION_PRIM(GetArrayLength) {
	jobject array = arg_jniref(c1);
	if (!checkIsArray(array, 0))
		return CHERI_FAIL;
	return (*env)->GetArrayLength(env, array);
}

#define ACCESS_ARRAY_ELEMENTS_COMMON(TYPE, jtype, ctype, perm) \
	jsize start = (jsize) a1, len = (jsize) a2; \
	jobject array_ref = arg_jniref(c1); \
	pObject array_obj = REF_TO_OBJ_WEAK_NULL_CHECK(array_ref); \
	if (!checkIsArray(array_ref, ctype)) \
		return CHERI_FAIL; \
	uintptr_t total_length = ARRAY_LEN(array_obj) * sizeof(jtype); \
	\
	size_t start_byte = start * sizeof(jtype); \
	size_t len_bytes = len * sizeof(jtype); \
	if (start_byte + len_bytes > total_length) \
		return CHERI_FAIL; \
	\
	__capability char *sandbox_buffer = arg_cap(c2, len_bytes, perm, TRUE); \
	char *host_buffer = ARRAY_DATA(array_obj, char); \
	if (sandbox_buffer == CNULL) \
		return CHERI_FAIL;


#define ACCESS_ARRAY_ELEMENTS(TYPE, jtype, ctype) \
	JNI_FUNCTION_PRIM(Get##TYPE##ArrayRegion) { \
		ACCESS_ARRAY_ELEMENTS_COMMON(TYPE, jtype, ctype, w) \
		copyToSandbox(sandbox_buffer, host_buffer + start_byte, len_bytes); \
		return CHERI_SUCCESS; \
	} \
	\
	JNI_FUNCTION_PRIM(Set##TYPE##ArrayRegion) { \
		ACCESS_ARRAY_ELEMENTS_COMMON(TYPE, jtype, ctype, r) \
		copyFromSandbox(host_buffer + start_byte, sandbox_buffer, len_bytes); \
		return CHERI_SUCCESS; \
	}

#define ARRAY_METHOD(op)   \
op(Boolean, jboolean, 'Z') \
op(Byte, jbyte, 'B')       \
op(Char, jchar, 'C')       \
op(Short, jshort, 'S')     \
op(Int, jint, 'I')         \
op(Long, jlong, 'J')       \
op(Float, jfloat, 'F')     \
op(Double, jdouble, 'D')

ARRAY_METHOD(ACCESS_ARRAY_ELEMENTS)

JNI_FUNCTION_CAP(GetDirectBufferAddress) {
	jobject buf = arg_jniref(c1);
	if (!checkIsValid(buf, Buffer))
		return CNULL;

	void *base = (*env)->GetDirectBufferAddress(env, buf);
	size_t length = (*env)->GetDirectBufferCapacity(env, buf);
	if (base == NULL || length == -1)
		return CNULL;

	// TODO: needs a type!
	return cheri_ptrperm(base, length, CHERI_PERM_LOAD | CHERI_PERM_STORE);
}

static inline int allowFileAccess(const char *path) {
	jam_printf("[ACCESS: %s => ALLOWED]\n", path);
	return TRUE;
}

LIBC_FUNCTION_CAP(GetStdinFD) {
	return return_fd(STDIN_FILENO);
}

LIBC_FUNCTION_CAP(GetStdoutFD) {
	return return_fd(STDOUT_FILENO);
}

LIBC_FUNCTION_CAP(GetStderrFD) {
	return return_fd(STDERR_FILENO);
}

LIBC_FUNCTION_CAP(GetStream) {
	int fd = arg_fd(c1);
	if (fd == -1)
		return CNULL;
	else if (fd == STDIN_FILENO)
		return return_file(stdin);
	else if (fd == STDOUT_FILENO)
		return return_file(stdout);
	else if (fd == STDERR_FILENO)
		return return_file(stderr);
	else
		return CNULL;
}

#define STAT_FUNCTION(NAME)                                                  \
	{                                                                        \
		const char *path = arg_str(c1, 0, r);                                \
		__capability struct stat *buf = arg_cap(c2, sizeof(struct stat), w, TRUE); \
		if (path == NULL || buf == CNULL)                                    \
			return CHERI_FAIL;                                               \
		                                                                     \
		if (!allowFileAccess(path))                                          \
			return CHERI_FAIL;                                               \
		                                                                     \
		struct stat data;                                                    \
		int res = NAME(path, &data);                                         \
		if (res < 0)                                                         \
			return CHERI_FAIL;                                               \
		                                                                     \
		buf[0] = data;                                                       \
		return CHERI_SUCCESS;                                                \
	}

LIBC_FUNCTION_PRIM(stat)
	STAT_FUNCTION(stat)

LIBC_FUNCTION_PRIM(lstat)
	STAT_FUNCTION(lstat)

LIBC_FUNCTION_PRIM(fstat) {
	int fd = arg_fd(c1);
	__capability struct stat *buf = arg_cap(c2, sizeof(struct stat), w, TRUE);
	if (fd < 0 || buf == CNULL)
		return CHERI_FAIL;

	struct stat data;
	int res = fstat(fd, &data);
	if (res < 0)
		return CHERI_FAIL;

	buf[0] = data;
	return CHERI_SUCCESS;
}

LIBC_FUNCTION_CAP(open) {
	const char *path = arg_str(c1, 0, r);
	int flags = a1;
	__capability int *fileno = arg_cap(c2, sizeof(int), w, TRUE);
	if (path == NULL || fileno == CNULL)
		return CNULL;

	if (flags & O_CREAT) // needs an extra argument
		return CNULL;

	if (!allowFileAccess(path))
		return CNULL;

	int fd = open(path, flags);

	fileno[0] = fd;
	return return_fd(fd);
}

LIBC_FUNCTION_PRIM(close) {
	int fd = arg_fd(c1);
	if (fd < 0)
		return CHERI_FAIL;

	int res = close(fd);
	if (res < 0)
		return CHERI_FAIL;
	else {
		revoke_fd(fd);
		return CHERI_SUCCESS;
	}
}

LIBC_FUNCTION_PRIM(read) {
	int fd = arg_fd(c1);
	__capability void *buf = arg_cap(c2, 1, w, TRUE);
	if (fd < 0 || buf == CNULL)
		return CHERI_FAIL;

	// TODO: should write directly into the capability

	void *buf_ptr = (void*) buf;
	size_t buf_len = cheri_getlen(buf);

	return (ssize_t) read(fd, buf_ptr, buf_len);
}

LIBC_FUNCTION_PRIM(write) {
	int fd = arg_fd(c1);
	__capability void *buf = arg_cap(c2, 1, r, TRUE);
	if (fd < 0 || buf == CNULL)
		return CHERI_FAIL;

	// TODO: should read directly from the capability

	void *buf_ptr = (void*) buf;
	size_t buf_len = cheri_getlen(buf);

	return (ssize_t) write(fd, buf_ptr, buf_len);
}

LIBC_FUNCTION_CAP(socket) {
	int domain = (int) a1;
	int type = (int) a2;
	int protocol = (int) a3;

	__capability int *fileno = arg_cap(c1, sizeof(int), w, TRUE);

	if (fileno == CNULL)
		return CNULL;

	int fd = socket(domain, type, protocol);

	printf("[SOCKET: opened new socket with fd=%d]\n", fd);

	fileno[0] = fd;
	return return_fd(fd);
}

register_t cherijni_trampoline(register_t methodnum, register_t a1, register_t a2, register_t a3, register_t a4, register_t a5, register_t a6, register_t a7, struct cheri_object system_object, __capability void *c1, __capability void *c2, __capability void *c3, __capability void *c4, __capability void *c5) __attribute__((cheri_ccall)) {
	switch(methodnum) {
	case CHERIJNI_JNIEnv_GetVersion:
		CALL_JNI_PRIM(GetVersion)
	case CHERIJNI_JNIEnv_DefineClass:
		break;
	case CHERIJNI_JNIEnv_FindClass:
		CALL_JNI_CAP(FindClass)
	case CHERIJNI_JNIEnv_FromReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_FromReflectedField:
		break;
	case CHERIJNI_JNIEnv_ToReflectedMethod:
		break;
	case CHERIJNI_JNIEnv_GetSuperclass:
		break;
	case CHERIJNI_JNIEnv_IsAssignableFrom:
		break;
	case CHERIJNI_JNIEnv_ToReflectedField:
		break;
	case CHERIJNI_JNIEnv_Throw:
		break;
	case CHERIJNI_JNIEnv_ThrowNew:
		CALL_JNI_PRIM(ThrowNew)
	case CHERIJNI_JNIEnv_ExceptionOccurred:
		CALL_JNI_CAP(ExceptionOccurred)
	case CHERIJNI_JNIEnv_ExceptionDescribe:
		CALL_JNI_PRIM(ExceptionDescribe)
	case CHERIJNI_JNIEnv_ExceptionClear:
		CALL_JNI_PRIM(ExceptionClear)
	case CHERIJNI_JNIEnv_FatalError:
		break;
	case CHERIJNI_JNIEnv_PushLocalFrame:
		CALL_JNI_PRIM(PushLocalFrame)
	case CHERIJNI_JNIEnv_PopLocalFrame:
		CALL_JNI_CAP(PopLocalFrame)
	case CHERIJNI_JNIEnv_NewGlobalRef:
		CALL_JNI_CAP(NewGlobalRef)
	case CHERIJNI_JNIEnv_DeleteGlobalRef:
		CALL_JNI_PRIM(DeleteGlobalRef)
	case CHERIJNI_JNIEnv_DeleteLocalRef:
		CALL_JNI_PRIM(DeleteLocalRef)
	case CHERIJNI_JNIEnv_IsSameObject:
		CALL_JNI_PRIM(IsSameObject)
	case CHERIJNI_JNIEnv_NewLocalRef:
		CALL_JNI_CAP(NewLocalRef)
	case CHERIJNI_JNIEnv_EnsureLocalCapacity:
		break;
	case CHERIJNI_JNIEnv_AllocObject:
		break;
	case CHERIJNI_JNIEnv_NewObject:
		break;
	case CHERIJNI_JNIEnv_GetObjectClass:
		break;
	case CHERIJNI_JNIEnv_IsInstanceOf:
		CALL_JNI_PRIM(IsInstanceOf)
	case CHERIJNI_JNIEnv_GetMethodID:
		CALL_JNI_CAP(GetMethodID)
	case CHERIJNI_JNIEnv_CallObjectMethod:
		CALL_JNI_CAP(CallObjectMethod)
	case CHERIJNI_JNIEnv_CallBooleanMethod:
		CALL_JNI_PRIM(CallBooleanMethod)
	case CHERIJNI_JNIEnv_CallByteMethod:
		CALL_JNI_PRIM(CallByteMethod)
	case CHERIJNI_JNIEnv_CallCharMethod:
		CALL_JNI_PRIM(CallCharMethod)
	case CHERIJNI_JNIEnv_CallShortMethod:
		CALL_JNI_PRIM(CallShortMethod)
	case CHERIJNI_JNIEnv_CallIntMethod:
		CALL_JNI_PRIM(CallIntMethod)
	case CHERIJNI_JNIEnv_CallLongMethod:
		CALL_JNI_PRIM(CallLongMethod)
	case CHERIJNI_JNIEnv_CallFloatMethod:
		CALL_JNI_PRIM(CallFloatMethod)
	case CHERIJNI_JNIEnv_CallDoubleMethod:
		CALL_JNI_PRIM(CallDoubleMethod)
	case CHERIJNI_JNIEnv_CallVoidMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallNonvirtualVoidMethod:
		break;
	case CHERIJNI_JNIEnv_GetFieldID:
		CALL_JNI_CAP(GetFieldID)
	case CHERIJNI_JNIEnv_GetObjectField:
		break;
	case CHERIJNI_JNIEnv_GetBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetByteField:
		break;
	case CHERIJNI_JNIEnv_GetCharField:
		break;
	case CHERIJNI_JNIEnv_GetShortField:
		break;
	case CHERIJNI_JNIEnv_GetIntField:
		break;
	case CHERIJNI_JNIEnv_GetLongField:
		break;
	case CHERIJNI_JNIEnv_GetFloatField:
		break;
	case CHERIJNI_JNIEnv_GetDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetObjectField:
		break;
	case CHERIJNI_JNIEnv_SetBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetByteField:
		break;
	case CHERIJNI_JNIEnv_SetCharField:
		break;
	case CHERIJNI_JNIEnv_SetShortField:
		break;
	case CHERIJNI_JNIEnv_SetIntField:
		break;
	case CHERIJNI_JNIEnv_SetLongField:
		break;
	case CHERIJNI_JNIEnv_SetFloatField:
		break;
	case CHERIJNI_JNIEnv_SetDoubleField:
		break;
	case CHERIJNI_JNIEnv_GetStaticMethodID:
		CALL_JNI_CAP(GetStaticMethodID)
	case CHERIJNI_JNIEnv_CallStaticObjectMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticBooleanMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticByteMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticCharMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticShortMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticIntMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticLongMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticFloatMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticDoubleMethod:
		break;
	case CHERIJNI_JNIEnv_CallStaticVoidMethod:
		break;
	case CHERIJNI_JNIEnv_GetStaticFieldID:
		break;
	case CHERIJNI_JNIEnv_GetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_GetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_GetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_GetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_GetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_GetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_GetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_GetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_GetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_SetStaticObjectField:
		break;
	case CHERIJNI_JNIEnv_SetStaticBooleanField:
		break;
	case CHERIJNI_JNIEnv_SetStaticByteField:
		break;
	case CHERIJNI_JNIEnv_SetStaticCharField:
		break;
	case CHERIJNI_JNIEnv_SetStaticShortField:
		break;
	case CHERIJNI_JNIEnv_SetStaticIntField:
		break;
	case CHERIJNI_JNIEnv_SetStaticLongField:
		break;
	case CHERIJNI_JNIEnv_SetStaticFloatField:
		break;
	case CHERIJNI_JNIEnv_SetStaticDoubleField:
		break;
	case CHERIJNI_JNIEnv_NewString:
		break;
	case CHERIJNI_JNIEnv_GetStringLength:
		break;
	case CHERIJNI_JNIEnv_GetStringChars:
		break;
	case CHERIJNI_JNIEnv_NewStringUTF:
		CALL_JNI_CAP(NewStringUTF)
	case CHERIJNI_JNIEnv_GetStringUTFLength:
		CALL_JNI_PRIM(GetStringUTFLength)
	case CHERIJNI_JNIEnv_GetStringUTFChars:
		CALL_JNI_PRIM(GetStringUTFChars)
	case CHERIJNI_JNIEnv_GetArrayLength:
		CALL_JNI_PRIM(GetArrayLength)
	case CHERIJNI_JNIEnv_NewObjectArray:
		break;
	case CHERIJNI_JNIEnv_GetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_SetObjectArrayElement:
		break;
	case CHERIJNI_JNIEnv_NewBooleanArray:
		break;
	case CHERIJNI_JNIEnv_NewByteArray:
		break;
	case CHERIJNI_JNIEnv_NewCharArray:
		break;
	case CHERIJNI_JNIEnv_NewShortArray:
		break;
	case CHERIJNI_JNIEnv_NewIntArray:
		break;
	case CHERIJNI_JNIEnv_NewLongArray:
		break;
	case CHERIJNI_JNIEnv_NewFloatArray:
		break;
	case CHERIJNI_JNIEnv_NewDoubleArray:
		break;
	case CHERIJNI_JNIEnv_GetBooleanArrayRegion:
		CALL_JNI_PRIM(GetBooleanArrayRegion)
	case CHERIJNI_JNIEnv_GetByteArrayRegion:
		CALL_JNI_PRIM(GetByteArrayRegion)
	case CHERIJNI_JNIEnv_GetCharArrayRegion:
		CALL_JNI_PRIM(GetCharArrayRegion)
	case CHERIJNI_JNIEnv_GetShortArrayRegion:
		CALL_JNI_PRIM(GetShortArrayRegion)
	case CHERIJNI_JNIEnv_GetIntArrayRegion:
		CALL_JNI_PRIM(GetIntArrayRegion)
	case CHERIJNI_JNIEnv_GetLongArrayRegion:
		CALL_JNI_PRIM(GetLongArrayRegion)
	case CHERIJNI_JNIEnv_GetFloatArrayRegion:
		CALL_JNI_PRIM(GetFloatArrayRegion)
	case CHERIJNI_JNIEnv_GetDoubleArrayRegion:
		CALL_JNI_PRIM(GetDoubleArrayRegion)
	case CHERIJNI_JNIEnv_SetBooleanArrayRegion:
		CALL_JNI_PRIM(SetBooleanArrayRegion)
	case CHERIJNI_JNIEnv_SetByteArrayRegion:
		CALL_JNI_PRIM(SetByteArrayRegion)
	case CHERIJNI_JNIEnv_SetCharArrayRegion:
		CALL_JNI_PRIM(SetCharArrayRegion)
	case CHERIJNI_JNIEnv_SetShortArrayRegion:
		CALL_JNI_PRIM(SetShortArrayRegion)
	case CHERIJNI_JNIEnv_SetIntArrayRegion:
		CALL_JNI_PRIM(SetIntArrayRegion)
	case CHERIJNI_JNIEnv_SetLongArrayRegion:
		CALL_JNI_PRIM(SetLongArrayRegion)
	case CHERIJNI_JNIEnv_SetFloatArrayRegion:
		CALL_JNI_PRIM(SetFloatArrayRegion)
	case CHERIJNI_JNIEnv_SetDoubleArrayRegion:
		CALL_JNI_PRIM(SetDoubleArrayRegion)
	case CHERIJNI_JNIEnv_RegisterNatives:
		break;
	case CHERIJNI_JNIEnv_UnregisterNatives:
		break;
	case CHERIJNI_JNIEnv_MonitorEnter:
		break;
	case CHERIJNI_JNIEnv_MonitorExit:
		break;
	case CHERIJNI_JNIEnv_GetJavaVM:
		break;
	case CHERIJNI_JNIEnv_GetStringRegion:
		break;
	case CHERIJNI_JNIEnv_GetStringUTFRegion:
		break;
	case CHERIJNI_JNIEnv_GetPrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_ReleasePrimitiveArrayCritical:
		break;
	case CHERIJNI_JNIEnv_GetStringCritical:
		break;
	case CHERIJNI_JNIEnv_ReleaseStringCritical:
		break;
	case CHERIJNI_JNIEnv_NewWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_DeleteWeakGlobalRef:
		break;
	case CHERIJNI_JNIEnv_ExceptionCheck:
		break;
	case CHERIJNI_JNIEnv_NewDirectByteBuffer:
		break;
	case CHERIJNI_JNIEnv_GetDirectBufferAddress:
		CALL_JNI_CAP(GetDirectBufferAddress)
	case CHERIJNI_JNIEnv_GetDirectBufferCapacity:
		break;
	case CHERIJNI_JNIEnv_GetObjectRefType:
		break;

	case CHERIJNI_LIBC_GetStdinFD:
		CALL_LIBC_CAP(GetStdinFD)
	case CHERIJNI_LIBC_GetStdoutFD:
		CALL_LIBC_CAP(GetStdoutFD)
	case CHERIJNI_LIBC_GetStderrFD:
		CALL_LIBC_CAP(GetStderrFD)
	case CHERIJNI_LIBC_GetStream:
		CALL_LIBC_CAP(GetStream)

	case CHERIJNI_LIBC_stat:
		CALL_LIBC_PRIM(stat)
	case CHERIJNI_LIBC_lstat:
		CALL_LIBC_PRIM(lstat)
	case CHERIJNI_LIBC_fstat:
		CALL_LIBC_PRIM(fstat)
	case CHERIJNI_LIBC_open:
		CALL_LIBC_CAP(open)
	case CHERIJNI_LIBC_close:
		CALL_LIBC_PRIM(close)
	case CHERIJNI_LIBC_read:
		CALL_LIBC_PRIM(read)
	case CHERIJNI_LIBC_write:
		CALL_LIBC_PRIM(write)

	case CHERIJNI_LIBC_socket:
		CALL_LIBC_CAP(socket)

	default:
		break;
	}

	return CHERI_FAIL;
}

void initialiseCheriJNI() {
	initVMReentrantLock(cherijni_sandbox_lock);

	cheri_system_user_register_fn(&cherijni_trampoline);
	INIT_SEAL(Reference);
	INIT_SEAL(MethodID);
	INIT_SEAL(FieldID);
	INIT_SEAL(FILE);

	int i;
	for (i = 0; i < FD_COUNT; i++)
		FDs[i] = cap_counter_new();

    class_String = findSystemClass0(SYMBOL(java_lang_String));
    class_Buffer = findSystemClass0(SYMBOL(java_nio_Buffer));
    registerStaticClassRef(&class_String);
    registerStaticClassRef(&class_Buffer);
}

#endif
