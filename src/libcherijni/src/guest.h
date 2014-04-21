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

extern struct cheri_object cherijni_SystemObject;

extern JavaVM *cherijni_getJavaVM();
extern JNIEnv *cherijni_getJNIEnv();
extern void cherijni_destroyJNIEnv(JNIEnv *ppEnv);
extern void cherijni_runTests(JNIEnv *env);

extern void cherijni_obj_init();
extern jobject cherijni_obj_storecap(__capability void *cobj);
extern __capability void *cherijni_obj_loadcap(jobject obj);

#endif //__GUEST_H__
