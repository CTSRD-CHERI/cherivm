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

#define cap_output                               (cheri_ptrperm(&cherijni_output, sizeof(__capability void*), CHERI_PERM_STORE | CHERI_PERM_STORE_CAP))

#define hostInvoke_3(name, a1, a2, a3)           hostInvoke_7(name, a1, a2, a3, 0, 0, 0, 0)
#define hostInvoke_2(name, a1, a2)               hostInvoke_3(name, a1, a2, 0)
#define hostInvoke_1(name, a1)                   hostInvoke_2(name, a1, 0)
#define hostInvoke_0(name)                       hostInvoke_1(name, 0)

#define check_cheri_fail(errcode, func_result)   { if (errcode == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return func_result; } }
#define check_cheri_fail_void(errcode)           { if (errcode == CHERI_FAIL) { printf("[SANDBOX ERROR: call to %s failed]\n", __func__); return; } }
#define get_output_obj                           cherijni_obj_storecap(cherijni_output)
#define get_output_str                           cherijni_extractHostString(cherijni_output)
#define return_obj(TYPE)                         return (TYPE) get_output_obj

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
