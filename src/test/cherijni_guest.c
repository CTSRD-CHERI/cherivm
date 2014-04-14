#include <sys/types.h>
#include <sys/stat.h>

#include <machine/cheri.h>
#include <machine/cheric.h>

#include <cheri/cheri_fd.h>
#include <cheri/cheri_invoke.h>
#include <cheri/cheri_memcpy.h>
#include <cheri/cheri_system.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int invoke(register_t methodnum,
           register_t a1, register_t a2, register_t a3, register_t a4,
           register_t a5, register_t a6, register_t a7,
           struct cheri_object system_object,
           __capability void *c5, __capability void *c6,
           __capability void *c7, __capability void *c8,
           __capability void *c9, __capability void *c10) {
	cheri_system_setup(system_object);
	printf("CHERI Sandbox: invoked op %d\n", (int) methodnum);
	return (-1);
}
