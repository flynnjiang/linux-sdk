#ifndef __SDK_TIMER_H__
#define __SDK_TIMER_H__

#include <pthread.h>

#include "sdk_list.h"
#include "sdk_types.h"

/* Timer's percision, in ms */
#define SDK_TIMER_PRECISION     10

typedef void (*sdk_timer_cb)(void *args);

struct sdk_timer_attr {
    unsigned int timeout;   // Timeout, ms
    int cycle;              // 是否循环计时。1-循环, 0-单次
    sdk_timer_cb cb_func;   // Callback funtions when timedout
    void *cb_data;
};


typedef struct sdk_timer_st {
    unsigned int cntdown;       // relative time
    int status;
#define SDK_TIMER_STATUS_STOP       0
#define SDK_TIMER_STATUS_START      1
#define SDK_TIMER_STATUS_TIMEOUT    2

    struct list_head list_node; // Inner use
    struct sdk_timer_attr attr; // Timer's attribute
} sdk_timer;


struct sdk_timer_mgt {
    int count;      // Count of all timers

    struct list_head stop_list;     // List of stopped timers
    struct list_head start_list;    // List of running timers
    struct list_head timeout_list;  // List of timedout timers

    pthread_mutex_t mutex;
};


/**************************************
 * Timer APIs
 **************************************/
sdk_ret sdk_timer_init();
sdk_ret sdk_timer_uninit();

sdk_timer *sdk_timer_create();
sdk_ret sdk_timer_destroy(sdk_timer *timer);

sdk_ret sdk_timer_get_attr(sdk_timer *timer, struct sdk_timer_attr *attr);
sdk_ret sdk_timer_set_attr(sdk_timer *timer, struct sdk_timer_attr *attr);

sdk_ret sdk_timer_start(sdk_timer *timer);
sdk_ret sdk_timer_stop(sdk_timer *timer);
sdk_ret sdk_timer_restart(sdk_timer *timer);
sdk_ret sdk_timer_timeout(sdk_timer *timer);

#endif /* __SDK_TIMER_H__ */
