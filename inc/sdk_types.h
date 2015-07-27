#ifndef __SDK_TYPES_H__
#define __SDK_TYPES_H__

#include <sys/types.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

/* SDK Return Values */
enum {
    SDK_OK = 0,
    SDK_EBADPARA,       // Bad Parameters
    SDK_ENOMEM,         // Memery week
    SDK_ESYS,           // Bad system call
    SDK_EIPC,           // IPC error
    SDK_EEXIST,         // Is already exist
    SDK_ENOTEXIST,      // Not exist
    SDK_EINNER          // Inner error
};
typedef int sdk_ret;

#endif /* __SDK_TYPES_H__ */
