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

#define check_cheri_fail(errcode, func_result)                { if (errcode == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return func_result; } }
#define check_cheri_fail_extra(errcode, func_result, doExtra) { if (errcode == CHERI_FAIL) { doExtra; check_cheri_fail(errcode, func_result) } }
#define check_cheri_fail_void(errcode)                        { if (errcode == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return; } }

#define get_obj(cap)        cherijni_obj_storecap(cap)
#define get_str(cap)        cherijni_extractHostString(cap)
#define get_cap(ptr)        (*((__capability void**) (ptr)))

extern JavaVM *cherijni_getJavaVM();
extern JNIEnv *cherijni_getJNIEnv(__capability void **context);
extern void cherijni_destroyJNIEnv(JNIEnv *ppEnv);
extern void cherijni_runTests(JNIEnv *env);
extern char* cherijni_extractHostString(__capability char* str_cap);

extern void cherijni_obj_init();
extern __capability void **cherijni_obj_storecap(__capability void *cobj);
extern __capability void *cherijni_obj_loadcap(__capability void **obj);

extern void cherijni_libc_init();

#endif //__GUEST_H__
