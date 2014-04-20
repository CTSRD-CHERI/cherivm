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

register_t cherijni_trampoline(register_t methodnum,
                               register_t a1, register_t a2, register_t a3, register_t a4,
                               register_t a5, register_t a6, register_t a7,
                               struct cheri_object system_object, __capability void *c3,
                               __capability void *c4, __capability void *c5,
                               __capability void *c6, __capability void *c7)
                               __attribute__((cheri_ccall)) {
	jam_printf("[CHERIJNI: trampoline op = %d]\n", methodnum);
	return (-1);
}

#endif
