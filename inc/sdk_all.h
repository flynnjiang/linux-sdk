#ifndef __SDK_ALL_H__
#define __SDK_ALL_H__

#include "sdk_mem.h"
#include "sdk_log.h"
#include "sdk_nv.h"
#include "sdk_timer.h"
#include "sdk_ipc.h"

static inline sdk_ret sdk_init(int module)
{
    sdk_ret sret = SDK_EINNER;

    sret = sdk_mem_init();
    if (SDK_OK != sret)
        return SDK_EINNER;

    sret = sdk_log_init();
    if (SDK_OK != sret)
        return SDK_EINNER;

    sret = sdk_nv_init();
    if (SDK_OK != sret)
        return SDK_EINNER;
    
    sret = sdk_ipc_init(module);
    if (SDK_OK != sret)
        return SDK_EINNER;
    
    sret = sdk_timer_init();
    if (SDK_OK != sret)
        return SDK_EINNER;    

    return SDK_OK;
}

static inline sdk_ret sdk_uninit()
{
    sdk_timer_uninit();
    sdk_ipc_uninit();
    sdk_nv_uninit();
    sdk_log_uninit();
    sdk_mem_uninit();

    return SDK_OK;
}

#endif /* __SDK_ALL_H__ */
