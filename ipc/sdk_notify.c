#include <string.h>

#include "sdk_mem.h"
#include "ipc_inner.h"

/* Global vars */
struct sdk_ipc_notify_mgt g_notify_mgt;


/* Function declare */
sdk_ret send_ipc_msg(sdk_ipc_msg *msg);



sdk_ret init_notify()
{
    /* Init notification register mgt */
    memset(&g_notify_mgt, 0, sizeof(g_notify_mgt));
    INIT_LIST_HEAD(&g_notify_mgt.list_head);
    pthread_mutex_init(&g_notify_mgt.mutex, NULL);

    return SDK_OK;
}

sdk_ret uninit_notify()
{
    struct list_head *pos = NULL, *tmp = NULL;
    struct sdk_ipc_notify_node *ntf = NULL;

    /* Destroy list */
    pthread_mutex_lock(&g_notify_mgt.mutex);
    list_for_each_safe(pos, tmp, &g_notify_mgt.list_head)
    {
        ntf = (struct sdk_ipc_notify_node *)list_entry(pos, \
                    struct sdk_ipc_notify_node, list_node);
        list_del(pos);
        g_notify_mgt.count--;

        sdk_mem_free(ntf);
        ntf = NULL;
    }
    pthread_mutex_unlock(&g_notify_mgt.mutex);

    pthread_mutex_destroy(&g_notify_mgt.mutex);

    return SDK_OK;
}



sdk_ret reg_notify(int module_id, int notify_id)
{
    struct sdk_ipc_notify_node *ntf = NULL, *tmp = NULL;
    int i = 0;
    sdk_ret sret = SDK_EINNER;

    pthread_mutex_lock(&g_notify_mgt.mutex);

    /* Find the notify's node */
    list_for_each_entry(tmp, &g_notify_mgt.list_head, list_node)
    {
        if (tmp->ntf_id == notify_id)
        {
            ntf = tmp;
            break;
        }
    }

    /* If node not exist, create & add one */
    if (NULL == ntf)
    {
        ntf = (struct sdk_ipc_notify_node *)sdk_mem_malloc(
                    sizeof(struct sdk_ipc_notify_node));
        if (NULL != ntf)
        {
            memset(ntf, 0, sizeof(struct sdk_ipc_notify_node));
            ntf->ntf_id = notify_id;
            list_add_tail(&ntf->list_node, &g_notify_mgt.list_head);
        }
        else
        {
            sret = SDK_ENOMEM;
        }
    }

    /* Register module to this notify */
    if (NULL != ntf)
    {
        /* Chekck if already registered */
        for (i = 0 ; i < sizeof(ntf->reg_array); i++)
        {
            if (ntf->reg_array[i] == module_id)
            {
                sret = SDK_EEXIST;
                break;
            }
        }

        /* Register */
        if (SDK_EEXIST != sret)
        {
            for (i = 0 ; i < sizeof(ntf->reg_array); i++)
            {
                if (ntf->reg_array[i] == 0)
                {
                    ntf->reg_array[i] = module_id;
                    sret = SDK_OK;
                    break;
                }
            }
        }
    }
    
    pthread_mutex_unlock(&g_notify_mgt.mutex);

    return sret;
}

sdk_ret dereg_notify(int module_id, int notify_id)
{
    struct sdk_ipc_notify_node *ntf = NULL, *tmp = NULL;
    sdk_ret sret = SDK_EINNER;
    int i = 0;

    pthread_mutex_lock(&g_notify_mgt.mutex);

    /* Find the notify's node */
    list_for_each_entry(tmp, &g_notify_mgt.list_head, list_node)
    {
        if (tmp->ntf_id == notify_id)
        {
            ntf = tmp;
            break;
        }
    }


    /* Deregister it */
    if (NULL != ntf)
    {
        for (i = 0; i < sizeof(ntf->reg_array); i++)
        {
            if (ntf->reg_array[i] == module_id)
            {
                ntf->reg_array[i] = 0;
                sret = SDK_OK;
                break;
            }
        }
    }
    else
    {
        sret = SDK_ENOTEXIST;
    }

    pthread_mutex_unlock(&g_notify_mgt.mutex);

    return sret;
}



sdk_ret sdk_ipc_reg_notify(int module_id, int notify_id)
{
    sdk_ipc_msg *req = NULL, *rsp = NULL;
    sdk_ret sret = SDK_EINNER;
    sdk_ipc_result *iret = NULL;

    req = sdk_ipc_msg_alloc(sizeof(int));
    if (NULL == req)
        return SDK_ENOMEM;

    req->hdr.id = SDK_INNER_MSG_REG_NTF_REQ;
    req->hdr.dest = module_id;
    *((int *)(req->data)) = notify_id;

    sret = sdk_ipc_request_syn(req, &rsp, 1000);
    if (SDK_OK != sret)
    {
        sdk_ipc_msg_free(req);
        return SDK_EINNER;
    }

    iret = (sdk_ipc_result *)(rsp->data);
    if (SDK_INNER_MSG_REG_NTF_RSP == rsp->hdr.id && SDK_IPC_OK == iret)
    {
        sret = SDK_OK;
    }
    else
    {
        sret = SDK_EINNER;
    }

    if (NULL != req)
        sdk_ipc_msg_free(req);

    if (NULL != rsp)
        sdk_ipc_msg_free(rsp);

    return sret;
}

sdk_ret sdk_ipc_dereg_notify(int module_id, int notify_id)
{
    sdk_ipc_msg *req = NULL, *rsp = NULL;
    sdk_ret sret = SDK_EINNER;
    sdk_ipc_result *iret = NULL;

    req = sdk_ipc_msg_alloc(sizeof(int));
    if (NULL == req)
        return SDK_ENOMEM;

    req->hdr.id = SDK_INNER_MSG_DEREG_NTF_REQ;
    req->hdr.dest = module_id;
    *((int *)(req->data)) = notify_id;

    sret = sdk_ipc_request_syn(req, &rsp, 1000);
    if (SDK_OK != sret)
    {
        sdk_ipc_msg_free(req);
        return SDK_EINNER;
    }

    iret = (sdk_ipc_result *)(rsp->data);
    if (SDK_INNER_MSG_DEREG_NTF_RSP == rsp->hdr.id && SDK_IPC_OK == iret)
    {
        sret = SDK_OK;
    }
    else
    {
        sret = SDK_EINNER;
    }

    if (NULL != req)
        sdk_ipc_msg_free(req);

    if (NULL != rsp)
        sdk_ipc_msg_free(rsp);

    return sret;
}


sdk_ret sdk_ipc_notify_signal(sdk_ipc_msg *notify)
{
    if (NULL == notify)
        return SDK_EBADPARA;

    notify->hdr.type = SDK_MSG_TYPE_NTF;

    return send_ipc_msg(notify);
}

sdk_ret sdk_ipc_notify_bdcast(sdk_ipc_msg *notify)
{
    struct sdk_ipc_notify_node *tmp = NULL;
    struct sdk_ipc_notify_node ntf;
    int i = 0;

    if (NULL == notify)
        return SDK_EBADPARA;

    /* Find the notify's node */
    pthread_mutex_lock(&g_notify_mgt.mutex);
    list_for_each_entry(tmp, &g_notify_mgt.list_head, list_node)
    {
        if (tmp->ntf_id == notify->hdr.id)
        {
            memset(&ntf, 0, sizeof(ntf));
            memcpy(&ntf, tmp, sizeof(ntf));
            break;
        }
    }
    pthread_mutex_unlock(&g_notify_mgt.mutex);

    if (ntf.ntf_id != notify->hdr.id)
        return SDK_ENOTEXIST;

    /* Boardcast it */
    notify->hdr.type = SDK_MSG_TYPE_NTF;
    
    for (i = 0; i < sizeof(ntf.reg_array); i++)
    {
        if (ntf.reg_array[i] != 0 )
        {
            notify->hdr.dest = ntf.reg_array[i];
            send_ipc_msg(notify);
        }
    }

    return SDK_OK;
}
