#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "sdk_mem.h"
#include "sdk_timer.h"

/* Gloable vars */
static int g_run_flag = 0;
static pthread_t g_thread_pid;
static struct sdk_timer_mgt g_timer_mgt;


sdk_ret start_timer(sdk_timer *timer)
{
    struct list_head *pos = NULL;
    sdk_timer *prev = NULL, *next = NULL;

    if (NULL == timer)
        return SDK_EBADPARA;

    if (SDK_TIMER_STATUS_START == timer->status)
        return SDK_OK;

    pthread_mutex_lock(&g_timer_mgt.mutex);

    /* Delete from current list */
    list_del(&timer->list_node);

    /* Find the right insert position */
    list_for_each(pos, &g_timer_mgt.start_list)
    {
        next = list_entry(pos, sdk_timer, list_node);
        if (timer->attr.timeout < next->attr.timeout)
            break;

        next = NULL;
    }

    /* Calculate the relative time */
    if (pos->prev != &g_timer_mgt.start_list)
    {
        prev = list_entry(pos->prev, sdk_timer, list_node);
        timer->cntdown = (timer->attr.timeout / SDK_TIMER_PRECISION) \
                        - (prev->attr.timeout / SDK_TIMER_PRECISION);
    }
    else
    {
        timer->cntdown = (timer->attr.timeout / SDK_TIMER_PRECISION);
    }

    if (NULL != next)
    {
        next->cntdown = (next->attr.timeout / SDK_TIMER_PRECISION ) \
                            - (timer->attr.timeout / SDK_TIMER_PRECISION);
    }

    /* Insert to start_list */
    __list_add(&timer->list_node, pos->prev, pos);
    timer->status = SDK_TIMER_STATUS_START;

    pthread_mutex_unlock(&g_timer_mgt.mutex);

    return SDK_OK;
}

sdk_ret stop_timer(sdk_timer *timer)
{
    sdk_timer *next = NULL;
    
    if (NULL == timer)
        return SDK_EBADPARA;

    if (SDK_TIMER_STATUS_STOP == timer->status)
        return SDK_OK;

    pthread_mutex_lock(&g_timer_mgt.mutex);
    
    /* Re-calculate next timer's cntdown */
    if (!list_is_last(&timer->list_node, &g_timer_mgt.start_list))
    {
        next = list_entry(timer->list_node.next, sdk_timer, list_node);
        next->cntdown += timer->cntdown;
    }

    /* Move to stop_list */
    timer->status = SDK_TIMER_STATUS_STOP;
    list_move(&timer->list_node, &g_timer_mgt.stop_list);

    pthread_mutex_unlock(&g_timer_mgt.mutex);

    return SDK_OK;
}

void *timer_thread_routine(void *arg)
{
    sdk_timer *timer = NULL;
    struct timeval tv;
    int ret = -1;

    while (1 == g_run_flag)
    {
        /* Wait timeout */        
        tv.tv_sec = 0;
        tv.tv_usec = SDK_TIMER_PRECISION * 1000;
        ret = select(0, NULL, NULL, NULL, &tv);
        if (ret < 0)
            continue;

        pthread_mutex_lock(&g_timer_mgt.mutex);

        if (list_empty(&g_timer_mgt.start_list))
        {
            pthread_mutex_unlock(&g_timer_mgt.mutex);            
            continue;
        }

        /* First timer Countdown */
        timer = list_first_entry(&g_timer_mgt.start_list, sdk_timer, list_node);
        timer->cntdown--;

        /* Move timedout timer to timeout_list */
        while (!list_empty(&g_timer_mgt.start_list))
        {
            timer = list_first_entry(&g_timer_mgt.start_list, sdk_timer, list_node);
            if (0 != timer->cntdown)
                break;

            timer->status = SDK_TIMER_STATUS_TIMEOUT;                
            list_move_tail(&timer->list_node, &g_timer_mgt.timeout_list);
        }

        pthread_mutex_unlock(&g_timer_mgt.mutex);

        /* Do timedout callback */
        while (!list_empty(&g_timer_mgt.timeout_list))
        {
            timer = list_first_entry(&g_timer_mgt.timeout_list, sdk_timer, list_node);

            timer->attr.cb_func(timer->attr.cb_data);
            timer->status = SDK_TIMER_STATUS_STOP;

            if (0 == timer->attr.cycle)
            {
                pthread_mutex_lock(&g_timer_mgt.mutex);
                list_move(&timer->list_node, &g_timer_mgt.stop_list);        
                pthread_mutex_unlock(&g_timer_mgt.mutex);
            }
            else
            {
                start_timer(timer);
            }
        }
    }

    return NULL;
}

sdk_ret sdk_timer_init()
{
    memset(&g_timer_mgt, 0, sizeof(g_timer_mgt));

    INIT_LIST_HEAD(&g_timer_mgt.start_list);
    INIT_LIST_HEAD(&g_timer_mgt.stop_list);
    INIT_LIST_HEAD(&g_timer_mgt.timeout_list);

    pthread_mutex_init(&g_timer_mgt.mutex, NULL);

    g_run_flag = 1;
    pthread_create(&g_thread_pid, NULL, timer_thread_routine, NULL);

    return SDK_OK;
}

sdk_ret sdk_timer_uninit()
{
    struct list_head *pos = NULL, *tmp = NULL;
    sdk_timer *timer = NULL;

    /* Stop thread */
    g_run_flag = 0;
    pthread_join(g_thread_pid, NULL);

    /* Destroy all timers */
    list_for_each_safe(pos, tmp, &g_timer_mgt.start_list)
    {
        timer = list_entry(pos, sdk_timer, list_node);
        list_del(pos);
        g_timer_mgt.count--;
        sdk_mem_free(timer);
    }

    list_for_each_safe(pos, tmp, &g_timer_mgt.stop_list)
    {
        timer = list_entry(pos, sdk_timer, list_node);
        list_del(pos);
        g_timer_mgt.count--;
        sdk_mem_free(timer);
    }

    list_for_each_safe(pos, tmp, &g_timer_mgt.timeout_list)
    {
        timer = list_entry(pos, sdk_timer, list_node);
        list_del(pos);
        g_timer_mgt.count--;
        sdk_mem_free(timer);
    }

    return SDK_OK;
}

sdk_timer *sdk_timer_create()
{
    sdk_timer *timer = NULL;

    timer = (sdk_timer *)sdk_mem_malloc(sizeof(sdk_timer));
    if (NULL == timer)
        return NULL;

    /* Add to stop list */
    memset(timer, 0, sizeof(sdk_timer));
    timer->status = SDK_TIMER_STATUS_STOP;

    pthread_mutex_lock(&g_timer_mgt.mutex);
    list_add(&timer->list_node, &g_timer_mgt.stop_list);
    g_timer_mgt.count++;
    pthread_mutex_unlock(&g_timer_mgt.mutex);
    
    return timer;
}

sdk_ret sdk_timer_destroy(sdk_timer *timer)
{

    if (NULL == timer)
        return SDK_EBADPARA;

    sdk_timer_stop(timer);

    pthread_mutex_lock(&g_timer_mgt.mutex);

    list_del(&timer->list_node);
    g_timer_mgt.count--;
    sdk_mem_free(timer);

    pthread_mutex_unlock(&g_timer_mgt.mutex);

    return SDK_OK;
}

sdk_ret sdk_timer_get_attr(sdk_timer *timer, struct sdk_timer_attr *attr)
{
    if (NULL == timer || NULL == attr)
        return SDK_EBADPARA;

    memcpy(attr, &timer->attr, sizeof(struct sdk_timer_attr));

    return SDK_OK;
}


sdk_ret sdk_timer_set_attr(sdk_timer *timer, struct sdk_timer_attr *attr)
{
    if (NULL == timer || NULL == attr)
        return SDK_EBADPARA;

    memcpy(&timer->attr, attr, sizeof(struct sdk_timer_attr));

    return SDK_OK;
}

sdk_ret sdk_timer_start(sdk_timer *timer)
{
    if (NULL == timer)
        return SDK_EBADPARA;

    return start_timer(timer);
}

sdk_ret sdk_timer_stop(sdk_timer *timer)
{
    if (NULL == timer)
        return SDK_EBADPARA;

    return stop_timer(timer);
}


sdk_ret sdk_timer_restart(sdk_timer *timer)
{
    if (NULL == timer)
        return SDK_EBADPARA;

    sdk_timer_stop(timer);
    sdk_timer_start(timer);

    return SDK_OK;
}

sdk_ret sdk_timer_timeout(sdk_timer *timer)
{

    /* FIXME: not thread safe */

    if (NULL == timer)
        return SDK_EBADPARA;

    if (SDK_OK != sdk_timer_stop(timer))
        return SDK_EINNER;

    timer->attr.cb_func(timer->attr.cb_data);

    if (1 == timer->attr.cycle)
        sdk_timer_start(timer);
    
    return SDK_OK;
}

