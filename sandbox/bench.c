#include <cheri/cheri.h>
struct cheri_object bench;

#include "bench_unsafe.c"

__attribute__((cheri_ccall))
int invoke(void)
{
	return (-1);
}


