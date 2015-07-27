#ifndef __IPC_INNER_H__
#define __IPC_INNER_H__

#include <pthread.h>

#include "sdk_list.h"
#include "sdk_ipc.h"

/* IPC Debug */
#ifdef SDK_DEBUG_IPC
 #include "sdk_log.h"
 #define SDK_LOG_TAG "SDK-IPC"
#else
 #define LOG_INFO(_fmt, _args...)
 #define LOG_WARN(_fmt, _args...)
 #define LOG_ERR(_fmt, _args...)
#endif /* SDK_DEBUG_IPC */


/* Max clients of a notification message can registered */
#define SDK_IPC_NTF_REG_MAX     8


/*******************************************
 * IPC INNER MESSAGES
 *******************************************/
#define SDK_INNER_MSG_REG_NTF_REQ       -1
/* Data Type: int */

#define SDK_INNER_MSG_REG_NTF_RSP       -2
/* Data Type: sdk_ipc_ret */

#define SDK_INNER_MSG_DEREG_NTF_REQ     -3
/* Data Type: int */

#define SDK_INNER_MSG_DEREG_NTF_RSP     -4
/* Data Type: sdk_ipc_ret */



/*******************************************
 * IPC Types
 *******************************************/

/* Message Queue Management */
struct sdk_ipc_mq_mgt {
    int count;                  // Count of mq

    struct list_head list_head; // mq list header
    pthread_mutex_t mutex;
};

/* SYNC Request Management */
struct sdk_ipc_sync_mgt {
    int count;
    struct list_head list_head; // List of sync_node
    pthread_mutex_t mutex;
};

/* SYNC Request */
struct sdk_ipc_sync_node {
    unsigned int sid;       // Session ID
    sdk_ipc_msg *rsp;       // Response message

    struct list_head list_node;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/* Notification Management */
struct sdk_ipc_notify_mgt {
    int count;
    struct list_head list_head; // List of notify_node
    pthread_mutex_t mutex;
};

struct sdk_ipc_notify_node {
    int ntf_id;         // Notification ID
    int reg_array[SDK_IPC_NTF_REG_MAX];   // Array of registered modules 
    struct list_head list_node;
};



#endif /* __IPC_INNER_H__ */
