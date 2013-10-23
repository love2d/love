#include <stdlib.h>

#include "wuff_config.h"

/* Default memory allocators. */
/* They can be overridden with custom functions at build time. */
#ifndef WUFF_MEMALLOC_OVERRIDE
void * wuff_alloc(size_t size)
{
	return malloc(size);
}

void wuff_free(void * mem)
{
	free(mem);
}
#endif
