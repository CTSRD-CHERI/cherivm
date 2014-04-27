#ifndef __GUEST_H__
#define __GUEST_H__

#include <sys/types.h>
#include <sys/stat.h>

#include <machine/cheri.h>
#include <machine/cheric.h>

#include <cheri/cheri_enter.h>
#include <cheri/cheri_fd.h>
#include <cheri/cheri_invoke.h>
#include <cheri/cheri_memcpy.h>
#include <cheri/cheri_system.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cherijni.h>
#include "sandbox_shared.h"

typedef FILE* pFILE;

extern struct cheri_object cherijni_obj_system;
extern __capability void *cherijni_output;

#define cap_output             (cheri_ptrperm(&cherijni_output, sizeof(__capability void*), CHERI_PERM_STORE | CHERI_PERM_STORE_CAP))
#define cap_buffer(ptr, len)   (cheri_ptrperm(ptr, len, CHERI_PERM_STORE))

#define hostInvoke_7_5(recast, name, a1, a2, a3, a4, a5, a6, a7, c1, c2, c3, c4, c5) \
	(((recast) cheri_invoke) (cherijni_obj_system, \
		hostInvoke_name(name), \
	    a1, a2, a3, a4, a5, a6, a7, \
	    c1, c2, c3, c4, c5, \
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap()))

#define hostInvoke_0_3(recast, name, c1, c2, c3)           hostInvoke_7_5(recast, name, 0, 0, 0, 0, 0, 0, 0, c1, c2, c3, CNULL, CNULL)
#define hostInvoke_0_2(recast, name, c1, c2)               hostInvoke_0_3(recast, name, c1, c2, CNULL)
#define hostInvoke_0_1(recast, name, c1)                   hostInvoke_0_2(recast, name, c1, CNULL)
#define hostInvoke_0_0(recast, name)                       hostInvoke_0_1(recast, name, CNULL)

#define check_cheri_fail(errcode, func_result)                { if ((errcode) == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return func_result; } }
#define check_cheri_fail_extra(errcode, func_result, doExtra) { if ((errcode) == CHERI_FAIL) { doExtra; check_cheri_fail(errcode, func_result) } }
#define check_cheri_fail_void(errcode)                        { if ((errcode) == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return; } }

#define get_str(cap)        cherijni_extractHostString(cap)
#define get_cap(ptr)        ((ptr) ? *((__capability void**) (ptr)) : CNULL)

extern JavaVM *cherijni_getJavaVM();
extern JNIEnv *cherijni_getJNIEnv(__capability void **context);
extern void cherijni_destroyJNIEnv(JNIEnv *ppEnv);
extern void cherijni_runTests(JNIEnv *env);
extern char* cherijni_extractHostString(__capability char* str_cap);

extern void cherijni_obj_init();
extern void cherijni_libc_init();

/*
 * !!! Types must always have the capability first !!!
 * For alignment reasons, and also checking whether a slot is being used.
 */
typedef __capability void* cherijni_objtype_jobject;
typedef __capability void* cherijni_objtype_jfieldID;
typedef struct {
	__capability void *cap;
	const char *sig;
} cherijni_objtype_jmethodID;
typedef struct {
	__capability void *cap;
	short fileno;               // the offset of this matches the offset inside FILE => fileno() works
} cherijni_objtype_pFILE;

extern jobject cherijni_jobject_store(__capability void *cap);
extern jfieldID cherijni_jfieldID_store(__capability void *cap);
extern jmethodID cherijni_jmethodID_store(__capability void *cap, const char *sig);
extern pFILE cherijni_pFILE_store(__capability void *cap);

#endif //__GUEST_H__
