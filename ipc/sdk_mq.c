
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include "sdk_mem.h"
#include "ipc_inner.h"


struct sdk_ipc_mq_mgt g_mq_mgt;


sdk_ret init_mq()
{
    /* Init message queue mgt */
    memset(&g_mq_mgt, 0, sizeof(g_mq_mgt));
    INIT_LIST_HEAD(&g_mq_mgt.list_head);
    pthread_mutex_init(&g_mq_mgt.mutex, NULL);

    return SDK_OK;
}


sdk_ret uninit_mq()
{
    sdk_ipc_mq *mq = NULL;
    struct list_head *pos = NULL, *tmp = NULL;

    /* Destroy all mq in mgt 
     * FIXME: op not in mutex
     */
    list_for_each_safe(pos, tmp, &g_mq_mgt.list_head)
    {
        mq = (sdk_ipc_mq *)list_entry(pos, sdk_ipc_mq, list_node);
        sdk_ipc_mq_destroy(mq);
    }

    pthread_mutex_destroy(&g_mq_mgt.mutex);

    return SDK_OK;
}


sdk_ipc_mq *sdk_ipc_mq_create()
{
    sdk_ipc_mq *mq = NULL;

    mq = (sdk_ipc_mq *)sdk_mem_malloc(sizeof(sdk_ipc_mq));
    if (NULL != mq)
    {
        /* Init mq */
        memset(mq, 0, sizeof(sdk_ipc_mq));
        INIT_LIST_HEAD(&mq->list_head);
        pthread_mutex_init(&mq->mutex, NULL);
        pthread_cond_init(&mq->cond, NULL);

        /* Add mq to mgt */
        pthread_mutex_lock(&g_mq_mgt.mutex);
        list_add_tail(&mq->list_node, &g_mq_mgt.list_head);
        g_mq_mgt.count++;
        pthread_mutex_unlock(&g_mq_mgt.mutex);
    }

    return mq;

}

sdk_ret sdk_ipc_mq_destroy(sdk_ipc_mq *mq)
{
    sdk_ipc_msg *msg = NULL;
    struct list_head *pos = NULL, *tmp = NULL;

    if (NULL == mq)
        return SDK_EBADPARA;

    /* Remove it from mgt */
    pthread_mutex_lock(&g_mq_mgt.mutex);
    list_del(&mq->list_node);
    g_mq_mgt.count--;
    pthread_mutex_unlock(&g_mq_mgt.mutex);
    
    /* Free messages in this mq */
    pthread_mutex_lock(&mq->mutex);
    list_for_each_safe(pos, tmp, &mq->list_head)
    {
        msg = (sdk_ipc_msg *)list_entry(pos, sdk_ipc_msg, list_node);
        list_del(pos);

        mq->count--;
        sdk_ipc_msg_free(msg);
    }
    pthread_mutex_unlock(&mq->mutex);

    /* Destroy mq */
    pthread_mutex_destroy(&mq->mutex);
    pthread_cond_destroy(&mq->cond);
    sdk_mem_free(mq);

    return SDK_OK;
}

sdk_ret sdk_ipc_mq_send(sdk_ipc_mq *mq, sdk_ipc_msg *msg)
{
    if (NULL == mq || NULL == msg)
        return SDK_EBADPARA;

    pthread_mutex_lock(&mq->mutex);
    list_add_tail(&msg->list_node, &mq->list_head);
    mq->count++;
    pthread_cond_signal(&mq->cond);
    pthread_mutex_unlock(&mq->mutex);

    return SDK_OK;
}

sdk_ipc_msg *sdk_ipc_mq_recv(sdk_ipc_mq *mq)
{
    sdk_ipc_msg *msg = NULL;

    if (NULL == mq)
        return NULL;

    pthread_mutex_lock(&mq->mutex);
    while (list_empty(&mq->list_head))
        pthread_cond_wait(&mq->cond, &mq->mutex);

    msg = (sdk_ipc_msg *)list_first_entry(&mq->list_head, sdk_ipc_msg, list_node);

    list_del(&msg->list_node);
    mq->count--;

    pthread_mutex_unlock(&mq->mutex);

    return msg;
}

sdk_ipc_msg *sdk_ipc_mq_recv_timeout(sdk_ipc_mq *mq, long ms)
{
    sdk_ipc_msg *msg = NULL;
    struct timeval now;
    struct timespec abstime;

    if (NULL == mq || ms < 0)
        return NULL;

    memset(&now, 0, sizeof(now));
    memset(&abstime, 0, sizeof(abstime));

    gettimeofday(&now, NULL);
    abstime.tv_sec = now.tv_sec + (ms / 1000);
    abstime.tv_nsec = (now.tv_usec + (ms % 1000 * 1000)) * 1000;

    pthread_mutex_lock(&mq->mutex);

    if (list_empty(&mq->list_head))
        pthread_cond_timedwait(&mq->cond, &mq->mutex, &abstime);

    if (!list_empty(&mq->list_head))
    {
        msg = (sdk_ipc_msg *)list_first_entry(&mq->list_head, sdk_ipc_msg, list_node);

        list_del(&msg->list_node);
        mq->count--;
    }

    pthread_mutex_unlock(&mq->mutex);

    return msg;
}

