#ifndef __SDK_IPC_H__
#define __SDK_IPC_H__


#include <pthread.h>

#include "sdk_list.h"
#include "sdk_types.h"

/* IPC Result */
typedef int sdk_ipc_result;
enum {
    SDK_IPC_OK,
    SDK_IPC_ERR
};


/* Max length of IPC messages */
#define SDK_IPC_MSG_MAX_LEN     1024


/* Message Header */
struct sdk_msg_hdr {
    int type;           // Message type
#define SDK_MSG_TYPE_INVALID    0
#define SDK_MSG_TYPE_REQ        1
#define SDK_MSG_TYPE_RSP        2
#define SDK_MSG_TYPE_NTF        3
#define SDK_MSG_TYPE_INNER      4

    int id;             // Message ID
    int dest;           // Destination address(module ID)
    int src;            // Source address(module ID)
    int data_len;       // Length of data
    unsigned int sid;   // Session ID, only for sync request
};

/* Message */
typedef struct sdk_ipc_msg_st {
    struct sdk_msg_hdr hdr;     // Message header
    struct list_head list_node; // Inner use
    void *data;                 // Message content
} sdk_ipc_msg;


/* Message Queue */
typedef struct sdk_ipc_mq_st {
    int count;                  // Count of message
    struct list_head list_head; // Msg list header
    struct list_head list_node; // Inner use
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sdk_ipc_mq;


/*************************************************
 * IPC APIs
 *************************************************/

sdk_ipc_msg *sdk_ipc_msg_alloc(size_t data_len);
sdk_ret sdk_ipc_msg_free(sdk_ipc_msg *msg);
sdk_ret sdk_ipc_msg_dump(sdk_ipc_msg *msg);

sdk_ipc_mq *sdk_ipc_mq_create();
sdk_ret sdk_ipc_mq_destroy(sdk_ipc_mq *mq);
sdk_ret sdk_ipc_mq_send(sdk_ipc_mq *mq, sdk_ipc_msg *msg);
sdk_ipc_msg *sdk_ipc_mq_recv(sdk_ipc_mq *mq);
sdk_ipc_msg *sdk_ipc_mq_recv_timeout(sdk_ipc_mq *mq, long ms);

sdk_ret sdk_ipc_init(int module_id);
sdk_ret sdk_ipc_uninit();

sdk_ret sdk_ipc_request_asy(sdk_ipc_msg *req);
sdk_ret sdk_ipc_request_syn(sdk_ipc_msg *req, sdk_ipc_msg **rsp, long timeout);
sdk_ret sdk_ipc_response(sdk_ipc_msg *req, sdk_ipc_msg *rsp);

sdk_ret sdk_ipc_reg_notify(int module_id, int notify_id);
sdk_ret sdk_ipc_dereg_notify(int module_id, int notify_id);
sdk_ret sdk_ipc_notify_signal(sdk_ipc_msg *notify);
sdk_ret sdk_ipc_notify_bdcast(sdk_ipc_msg *notify);

sdk_ipc_msg *sdk_ipc_wait_msg();
sdk_ipc_msg *sdk_ipc_wait_msg_timeout(long ms);



#endif /* __SDK_IPC_H__ */
