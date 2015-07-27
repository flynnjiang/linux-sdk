#include <string.h>
#include <sys/time.h>

#include "sdk_mem.h"
#include "ipc_inner.h"

struct sdk_ipc_sync_mgt g_sync_mgt;


sdk_ret init_sync()
{
    /* Init sync request mgt */
    memset(&g_sync_mgt, 0, sizeof(g_sync_mgt));
    INIT_LIST_HEAD(&g_sync_mgt.list_head);
    pthread_mutex_init(&g_sync_mgt.mutex, NULL);

    return SDK_OK;
}

sdk_ret uninit_sync()
{
    struct sdk_ipc_sync_node *sync = NULL;
    struct list_head *pos = NULL, *tmp = NULL;

    /* Free sync list */
    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_for_each_safe(pos, tmp, &g_sync_mgt.list_head)
    {
        sync = (struct sdk_ipc_sync_node *)list_entry(pos, struct sdk_ipc_sync_node, list_node);
        list_del(pos);
        g_sync_mgt.count--;

        if (sync->rsp)
            sdk_ipc_msg_free(sync->rsp);

        pthread_mutex_destroy(&sync->mutex);
        pthread_cond_destroy(&sync->cond);

        sdk_mem_free(sync);
    }
    pthread_mutex_unlock(&g_sync_mgt.mutex);    

    pthread_mutex_destroy(&g_sync_mgt.mutex);

    return SDK_OK;
}


sdk_ret reg_sync_request(unsigned int sid)
{
    struct sdk_ipc_sync_node *sync = NULL;

    /* Check if already exist */
    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_for_each_entry(sync, &g_sync_mgt.list_head, list_node)
    {
        if (sync->sid == sid)
        {
            pthread_mutex_unlock(&g_sync_mgt.mutex);
            return SDK_EEXIST;
        }
    }
    pthread_mutex_unlock(&g_sync_mgt.mutex);

    /* Create sync node */
    sync = (struct sdk_ipc_sync_node *)sdk_mem_malloc(sizeof(struct sdk_ipc_sync_node));
    if (NULL == sync)
        return SDK_ENOMEM;

    memset(sync, 0, sizeof(struct sdk_ipc_sync_node));
    sync->sid = sid;
    sync->rsp = NULL;
    pthread_mutex_init(&sync->mutex, NULL);
    pthread_cond_init(&sync->cond, NULL);

    /* Add to sync mgt */
    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_add(&sync->list_node, &g_sync_mgt.list_head);
    g_sync_mgt.count++;
    pthread_mutex_unlock(&g_sync_mgt.mutex);

    return SDK_OK;
}

sdk_ret dereg_sync_request(unsigned int sid)
{
    struct sdk_ipc_sync_node *sync = NULL;
    sdk_ret sret = SDK_EINNER;

    /* Check if exist & del it */
    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_for_each_entry(sync, &g_sync_mgt.list_head, list_node)
    {
        if (sync->sid == sid)
        {
            list_del(&sync->list_node);
            sret = SDK_OK;
            break;
        }
    }
    pthread_mutex_unlock(&g_sync_mgt.mutex);

    /* Destroy it */
    if (SDK_OK == sret)
    {
        pthread_mutex_destroy(&sync->mutex);
        pthread_cond_destroy(&sync->cond);

        /* If response exist, drop it */
        if (NULL != sync->rsp)
        {
            sdk_mem_free(sync->rsp);
            sync->rsp = NULL;
        }

        sdk_mem_free(sync);
    }

    return sret;
}

struct sdk_ipc_sync_node *get_sync_request(unsigned int sid)
{
    struct sdk_ipc_sync_node *sync = NULL;
    struct sdk_ipc_sync_node *ret = NULL;

    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_for_each_entry(sync, &g_sync_mgt.list_head, list_node)
    {
        if (sync->sid == sid)
        {
            ret = sync;
            break;
        }
    }
    pthread_mutex_unlock(&g_sync_mgt.mutex);

    return ret;
}


sdk_ret try_put_sync_response(sdk_ipc_msg *rsp)
{
    struct sdk_ipc_sync_node *sync = NULL;
    sdk_ret sret = SDK_EINNER;

    if (NULL == rsp || SDK_MSG_TYPE_RSP != rsp->hdr.type)
        return SDK_EBADPARA;

    sret = SDK_ENOTEXIST;
    pthread_mutex_lock(&g_sync_mgt.mutex);
    list_for_each_entry(sync, &g_sync_mgt.list_head, list_node)
    {
        if (sync->sid == rsp->hdr.sid)
        {
            pthread_mutex_lock(&sync->mutex);
            sync->rsp = rsp;
            pthread_cond_signal(&sync->cond);
            pthread_mutex_unlock(&sync->mutex);
            sret = SDK_OK;

            break;
        }
    }

    pthread_mutex_unlock(&g_sync_mgt.mutex);    

    return sret;
}

sdk_ipc_msg *wait_sync_response(unsigned int sid, long timeout)
{
    struct timeval now;
    struct timespec abstime;
    struct sdk_ipc_sync_node *sync = NULL;
    sdk_ipc_msg *rsp = NULL;

    sync = get_sync_request(sid);
    if (NULL == sync)
        return NULL;

    /* Waiting for the response */
    gettimeofday(&now, NULL);
    abstime.tv_sec = now.tv_sec + (timeout / 1000);
    abstime.tv_nsec = (now.tv_usec + (timeout % 1000 * 1000)) * 1000;

    pthread_mutex_lock(&sync->mutex);
    if (NULL == sync->rsp)
        pthread_cond_timedwait(&sync->cond, &sync->mutex, &abstime);

    /* Got it or timeout */
    rsp = sync->rsp;
    sync->rsp = NULL;
    pthread_mutex_unlock(&g_sync_mgt.mutex);

    /* Deregitster sync info */
    dereg_sync_request(sid);

    return rsp;
}
