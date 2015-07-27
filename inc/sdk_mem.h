#ifndef __SDK_MEM_H__
#define __SDK_MEM_H__

#include "sdk_types.h"

sdk_ret sdk_mem_init();
sdk_ret sdk_mem_uninit();

void *sdk_mem_malloc(size_t size);
void *sdk_mem_calloc(size_t num, size_t size);
void *sdk_mem_realloc(void *ptr, size_t size);
sdk_ret sdk_mem_free(void *ptr);
sdk_ret sdk_mem_dump();

#endif /* __SDK_MEM_H__ */
