#include <stdlib.h>

#include "mem_inner.h"

sdk_ret sdk_mem_init()
{
    return SDK_OK;
}

sdk_ret sdk_mem_uninit()
{
    return SDK_OK;
}

void *sdk_mem_malloc(size_t size)
{
    return malloc(size);
}

void *sdk_mem_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void *sdk_mem_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

sdk_ret sdk_mem_free(void *ptr)
{
    free(ptr);

    return SDK_OK;
}

sdk_ret sdk_mem_dump()
{
    return SDK_OK;
}
