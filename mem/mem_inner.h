#ifndef __MEM_INNER_H__
#define __MEM_INNER_H__


#include "sdk_mem.h"

#ifdef SDK_DEBUG_MEM
#define MEMWATCH
#define MEMWATCH_STDIO
# include "memwatch.h"
#endif /* SDK_DEBUG_MEM */


#endif /* __MEM_INNER_H__ */
