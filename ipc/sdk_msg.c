#include <string.h>

#include "sdk_mem.h"
#include "ipc_inner.h"

sdk_ipc_msg *sdk_ipc_msg_alloc(size_t data_len)
{
    sdk_ipc_msg *msg = NULL;

    msg = (sdk_ipc_msg *)sdk_mem_malloc(sizeof(sdk_ipc_msg) + data_len);
    if (NULL != msg)
    {
        memset(msg, 0, sizeof(sdk_ipc_msg));

        if (data_len > 0)
        {
            msg->hdr.data_len = data_len;
            msg->data = msg + 1;
        }
    }

    return msg;
}


sdk_ret sdk_ipc_msg_free(sdk_ipc_msg *msg)
{
    if (NULL == msg)
        return SDK_EBADPARA;

    return sdk_mem_free(msg);
}


sdk_ret sdk_ipc_msg_dump(sdk_ipc_msg *msg)
{
    return SDK_OK;
}

