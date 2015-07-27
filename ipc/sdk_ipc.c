#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include "sdk_mem.h"
#include "ipc_inner.h"



/***********************************************
 * Gloable Vars
 **********************************************/
static sdk_ipc_mq *g_main_mq = NULL;
static int g_run_flag = 0;
static int g_init_flag = 0;
static int g_self_id = -1;
static int g_sockfd = -1;
static pthread_t g_recv_pid;


/***********************************************
 * Function Declares
 **********************************************/
sdk_ret init_mq();
sdk_ret uninit_mq();

sdk_ret init_sync();
sdk_ret uninit_sync();

sdk_ret init_notify();
sdk_ret uninit_notify();

sdk_ret init_socket();
sdk_ret uninit_socket();

void *recv_thread_routine(void *arg);
sdk_ret get_sockaddr_un(int module_id, struct sockaddr_un *addr);

sdk_ret reg_sync_request(unsigned int sid);
sdk_ret dereg_sync_request(unsigned int sid);
sdk_ret try_put_sync_response(sdk_ipc_msg *rsp);
sdk_ipc_msg *wait_sync_response(unsigned int sid, long timeout);

sdk_ret reg_notify(int module_id, int notify_id);
sdk_ret dereg_notify(int module_id, int notify_id);


/***********************************************
 * Functions Definitions
 **********************************************/

sdk_ret get_sockaddr_un(int module_id, struct sockaddr_un *addr)
{
    if (NULL == addr)
        return SDK_EBADPARA;

    addr->sun_family = AF_UNIX;
#if 1
    addr->sun_path[0] = '\0';
    snprintf(addr->sun_path + 1, sizeof(addr->sun_path) - 1,
                "sdk_ipc_addr_%d", module_id);
#else
    snprintf(addr->sun_path, sizeof(addr->sun_path),
                "/tmp/sdk_ipc_addr_%d", module_id);
#endif

    return SDK_OK;
}

sdk_ret init_socket()
{
    struct sockaddr_un addr;
    int ret = -1;

    if (g_sockfd > 0)
    {
        LOG_ERR("socket has been opened before!");
        return SDK_EINNER;
    }

    g_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (g_sockfd < 0)
    {
        LOG_ERR("open socket failed! errno = %d", errno);
        return SDK_ESYS;
    }

    memset(&addr, 0, sizeof(addr));
    get_sockaddr_un(g_self_id, &addr);
    ret = bind(g_sockfd, (struct sockaddr *)&addr , sizeof(addr));
    if (ret < 0)
    {
        close(g_sockfd);
        g_sockfd = -1;
        LOG_ERR("bind socket addr failed! errno = %d", errno);
        return SDK_ESYS;
    }

    return SDK_OK;
}

sdk_ret uninit_socket()
{
    int ret = -1;

    if (g_sockfd < 0)
        return SDK_EINNER;

    ret = close(g_sockfd);
    if (ret < 0)
        return SDK_ESYS;

    g_sockfd = -1;

    return SDK_OK;
}



sdk_ret send_ipc_msg(sdk_ipc_msg *msg)
{
    struct sockaddr_un addr;
    size_t msg_len = 0;
    int ret = -1;

    if (NULL == msg)
        return SDK_EBADPARA;

    LOG_INFO("IPC send msg, type=%d, id=%d", msg->hdr.type, msg->hdr.id);    

    /* Make sure the src is myself. */
    msg->hdr.src = g_self_id;

    memset(&addr, 0, sizeof(addr));
    get_sockaddr_un(msg->hdr.dest, &addr);

    msg_len = sizeof(sdk_ipc_msg) + msg->hdr.data_len;
    ret = sendto(g_sockfd, msg, msg_len, 0,
            (struct sockaddr *)&addr, sizeof(addr));
    if (ret != msg_len)
    {
        LOG_ERR("call sendto() failed, ret = %d, errno = %d", ret, errno);
        return SDK_ESYS;
    }

    return SDK_OK;
}

sdk_ipc_msg *recv_ipc_msg(struct timeval *timeout)
{
    char buf[SDK_IPC_MSG_MAX_LEN];
    struct sockaddr_un addr = {0};
    socklen_t addr_len = 0;
    sdk_ipc_msg *msg = NULL;
    fd_set rfds;
    int ret = 0;
    ssize_t msg_len = 0;

    FD_ZERO(&rfds);
    FD_SET(g_sockfd, &rfds);
    ret = select(g_sockfd+1, &rfds, NULL, NULL, timeout);
    if (ret <= 0)   // error or timeout
        return NULL;

    memset(buf, 0, sizeof(buf));
    memset(&addr, 0, sizeof(addr));
    addr_len = 0;
    ret = recvfrom(g_sockfd, buf, sizeof(buf), 0,
                (struct sockaddr *)&addr, &addr_len);
    if (ret <= 0)    // error or socket closed
    {
        LOG_ERR("call recvfrom() failed, ret = %d, errno= %d", ret, errno);
        return NULL;
    }

    LOG_INFO("Socket recv data, len = %d", ret);

    /* Check Msg length */
    msg_len = sizeof(sdk_ipc_msg) + ((sdk_ipc_msg *)buf)->hdr.data_len;
    if (ret < sizeof(sdk_ipc_msg) || ret != msg_len)
    {
        LOG_WARN("Invalid msg, drop it!");
        return NULL;
    }

    /* Check Msg src */
    // Do nothing

    /* Build Msg */
    msg = (sdk_ipc_msg *)sdk_mem_malloc(msg_len);
    if (NULL != msg)
    {
        memset(msg, 0, msg_len);
        memcpy(msg, buf, msg_len);
        msg->data = msg + 1;
    }

    return msg;
}

sdk_ret handle_reg_notify_msg(sdk_ipc_msg *msg)
{
    int ntf_id = -1;
    sdk_ipc_msg *rsp = NULL;
    sdk_ret sret = SDK_EINNER;

    rsp = sdk_ipc_msg_alloc(sizeof(sdk_ipc_result));
    if (NULL == rsp)
        return SDK_ENOMEM;

    ntf_id = *((int *)(msg->data));
    sret = reg_notify(msg->hdr.src, ntf_id);

    rsp->hdr.id = SDK_INNER_MSG_REG_NTF_RSP;
    *((sdk_ipc_result *)(rsp->data)) = (SDK_OK == sret ? \
                SDK_IPC_OK : SDK_IPC_ERR);

    sdk_ipc_response(msg, rsp);

    sdk_ipc_msg_free(rsp);

    return SDK_OK;
}

sdk_ret handle_dereg_notify_msg(sdk_ipc_msg *msg)
{
    int ntf_id = -1;
    sdk_ipc_msg *rsp = NULL;
    sdk_ret sret = SDK_EINNER;

    rsp = sdk_ipc_msg_alloc(sizeof(sdk_ipc_result));
    if (NULL == rsp)
        return SDK_ENOMEM;

    ntf_id = *((int *)(msg->data));
    sret = dereg_notify(msg->hdr.src, ntf_id);

    rsp->hdr.id = SDK_INNER_MSG_REG_NTF_RSP;
    *((sdk_ipc_result *)(rsp->data)) = (SDK_OK == sret ? \
                SDK_IPC_OK : SDK_IPC_ERR);

    sdk_ipc_response(msg, rsp);

    sdk_ipc_msg_free(rsp);

    return SDK_OK;
}


sdk_ret handle_inner_msg(sdk_ipc_msg *msg)
{
    sdk_ret sret = SDK_EINNER;

    if (NULL == msg || msg->hdr.id >= 0)
        return SDK_EBADPARA;

    switch (msg->hdr.id)
    {
        case SDK_INNER_MSG_REG_NTF_REQ:
            sret = handle_reg_notify_msg(msg);
            break;

        case SDK_INNER_MSG_DEREG_NTF_REQ:
            sret = handle_dereg_notify_msg(msg);
            break;

        default:
            break;
    }

    return sret;
}

void *recv_thread_routine(void *arg)
{
    sdk_ipc_msg *msg = NULL;
    struct timeval tv;

    /* Set timeout to 2s */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while(1 == g_run_flag)
    {
        msg = recv_ipc_msg(&tv);
        if (NULL == msg)
            continue;

        LOG_INFO("IPC thread recv msg, type=%d, id=%d", \
                    msg->hdr.type, msg->hdr.id);

        /* If sync rsp, put it into sync queue */
        if (SDK_MSG_TYPE_RSP == msg->hdr.type)
        {
            if (SDK_OK == try_put_sync_response(msg))
            {
                LOG_INFO("Put msg into sync_mgt");
                continue;
            }
        }

        /* Handle other inner msg (except sync rsp) */
        if (msg->hdr.id < 0)
        {
            LOG_INFO("Handled inner msg.");
            handle_inner_msg(msg);
            sdk_ipc_msg_free(msg);
            continue;
        }

        /* Put into IPC main mq */
        LOG_INFO("Put msg into main mq");
        sdk_ipc_mq_send(g_main_mq, msg);
    }

    return NULL;
}


sdk_ret sdk_ipc_init(int module_id)
{
    sdk_ret sret = SDK_EINNER;
    int ret = -1;

    if (g_init_flag)
        return SDK_EBADPARA;

    g_self_id = module_id;

    sret = init_mq();
    if (SDK_OK != sret)
        goto err_mq;

    sret = init_sync();
    if (SDK_OK != sret)
        goto err_sync;

    sret = init_notify();
    if (SDK_OK != sret)
        goto err_notify;
    
    sret = init_socket();
    if (SDK_OK != sret)
        goto err_sock;

    /* Create IPC main MQ */
    g_main_mq = sdk_ipc_mq_create();
    if (NULL == g_main_mq)
        goto  err_sock;

    /* Start IPC recv thread */
    g_run_flag = 1;
    ret = pthread_create(&g_recv_pid, NULL, recv_thread_routine, NULL);
    if (0 != ret)
        goto err_sock;

    /* Init OK */
    g_init_flag = 1;

    return SDK_OK;

err_sock:
    uninit_socket();

err_notify:
    uninit_notify();

err_sync:
    uninit_sync();

err_mq:
    uninit_mq();

    return SDK_EINNER;
}

sdk_ret sdk_ipc_uninit()
{
    int ret = -1;

    /* Stop recv thread */
    g_run_flag = 0;
    ret = pthread_join(g_recv_pid, NULL);
    if (ret < 0)
    {
        /* Nothing */
    }

    uninit_socket();

    uninit_mq();

    uninit_sync();

    uninit_notify();

    g_init_flag = 0;

    return SDK_OK;
}


sdk_ret sdk_ipc_request_asy(sdk_ipc_msg *req)
{
    if (NULL == req)
        return SDK_EBADPARA;

    req->hdr.type = SDK_MSG_TYPE_REQ;
    req->hdr.src = g_self_id;

    return send_ipc_msg(req);
}

sdk_ret sdk_ipc_request_syn(sdk_ipc_msg *req, sdk_ipc_msg **rsp, long timeout)
{
    static unsigned int session_id = 0;     /* self increase */
    unsigned int curr_sid = 0;
    sdk_ipc_msg *msg = NULL;
    sdk_ret sret = SDK_EINNER;
    
    if (NULL == req)
        return SDK_EBADPARA;

    curr_sid = session_id++;

    /* Register sync request info */
    sret = reg_sync_request(curr_sid);
    if (SDK_OK != sret)
        return SDK_EINNER;

    /* Send request */
    req->hdr.type = SDK_MSG_TYPE_REQ;
    req->hdr.src = g_self_id;
    req->hdr.sid = curr_sid;

    sret = send_ipc_msg(req);
    if (SDK_OK != sret)
    {
        dereg_sync_request(curr_sid);
        return SDK_EINNER;
    }

    /* Wait the response */
    msg = wait_sync_response(curr_sid, timeout);
    if (NULL == msg)
        return SDK_EINNER;

    if (NULL != rsp) {
        *rsp = msg;
    } else {
        sdk_ipc_msg_free(msg);
    }

    return SDK_OK;
}


sdk_ret sdk_ipc_response(sdk_ipc_msg *req, sdk_ipc_msg *rsp)
{
    if (NULL == req || NULL == rsp)
        return SDK_EBADPARA;

    rsp->hdr.type = SDK_MSG_TYPE_RSP;
    rsp->hdr.dest = req->hdr.src;
    rsp->hdr.src = g_self_id;
    rsp->hdr.sid = req->hdr.sid;

    return send_ipc_msg(rsp);
}


sdk_ipc_msg *sdk_ipc_wait_msg()
{
    if (NULL == g_main_mq)
        return NULL;

    return sdk_ipc_mq_recv(g_main_mq);
}

sdk_ipc_msg *sdk_ipc_wait_msg_timeout(long ms)
{
    if (NULL == g_main_mq || ms < 0)
        return NULL;

    return sdk_ipc_mq_recv_timeout(g_main_mq, ms);
}

