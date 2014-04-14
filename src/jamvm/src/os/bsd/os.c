/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pthread.h>

#ifndef __NetBSD__
#include <pthread_np.h>
#endif

#include "../../jam.h"

#ifdef JNI_CHERI
#include <machine/cheri.h>
#include <machine/cheric.h>
#include <machine/cpuregs.h>

#include <cheri/sandbox.h>
#endif

#ifdef __OpenBSD__
void *nativeStackBase() {
    stack_t sinfo;

    pthread_stackseg_np(pthread_self(), &sinfo);
    return sinfo.ss_sp;
}
#else
void *nativeStackBase() {
    pthread_attr_t attr;
    size_t size;
    void *addr;

    pthread_attr_init(&attr);
    pthread_attr_get_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &addr, &size);

    return addr+size;
}
#endif

int nativeAvailableProcessors() {
    int processors, mib[2];
    size_t len = sizeof(processors);

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;

    if(sysctl(mib, 2, &processors, &len, NULL, 0) == -1)
        return 1;
    else
        return processors;
}

char *nativeLibPath() {
    return getenv("LD_LIBRARY_PATH");
}

void nativeLibClose(void *handle) {
    dlclose(handle);
}

#ifndef JNI_CHERI

char *nativeLibError() {
    return dlerror();
}

void *nativeLibOpen(char *path) {
    return dlopen(path, RTLD_LAZY);
}

char *nativeLibMapName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof("lib.so") + 1);

   sprintf(buff, "lib%s.so", name);
   return buff;
}

void *nativeLibSym(void *handle, char *symbol) {
    return dlsym(handle, symbol);
}

#else // JNI_CHERI

struct cherijni_sandbox {
	struct sandbox_class    *classp;
	struct sandbox_object   *objectp;
};

#define CHERIJNI_METHOD_RESOLVE		0

char *nativeLibError() {
    return NULL;
}

void *nativeLibOpen(char *path) {
	/*
	 *	Initialize the sandbox class and object
	 */

	struct cherijni_sandbox *sandbox = sysMalloc(sizeof(struct cherijni_sandbox));

	if (sandbox_class_new(path, DEFAULT_SANDBOX_MEM, &sandbox->classp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	if (sandbox_object_new(sandbox->classp, &sandbox->objectp) < 0) {
		sysFree(sandbox);
		return NULL;
	}

	return sandbox;
}

char *nativeLibMapName(char *name) {
   char *buff = sysMalloc(strlen(name) + sizeof("CHERI_.bin") + 1);

   sprintf(buff, "CHERI_%s.bin", name);
   return buff;
}

void *nativeLibSym(void *handle, char *method) {
	jam_printf("CheriJni: Looking up %s\n", method);

	struct cherijni_sandbox *sandbox = (struct cherijni_sandbox *) handle;

	__capability void *cclear = cheri_zerocap();

	register_t res = sandbox_object_cinvoke(
            sandbox->objectp,
            CHERIJNI_METHOD_RESOLVE,
            0, 0, 0, 0, 0, 0, 0,
            sandbox_object_getsystemobject(sandbox->objectp).co_codecap,
            sandbox_object_getsystemobject(sandbox->objectp).co_datacap,
            cclear, cclear,
            cclear, cclear,
            cclear, cclear);

	return NULL;
}

#endif // JNI_CHERI
