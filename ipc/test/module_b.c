#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "sdk_ipc.h"
#include "m_def.h"


static int run_flag = 0;
static pthread_t thread_pid;


void *thread_routine(void *args)
{
    int c = -1;
    sdk_ipc_msg *req = NULL, *rsp = NULL;

    while (c = getchar())
    {
        getchar();
        printf("choose = %c\n", c);
        switch (c)
        {
            case '0':
                req = sdk_ipc_msg_alloc(sizeof(struct msg_data));
                req->hdr.id = MSG_REQ;
                req->hdr.dest = MA;
                strcpy(req->data, "reqeust from mb");
                sdk_ipc_request_syn(req, &rsp, 1000);
                print_msg(rsp);
                sdk_ipc_msg_free(req);
                sdk_ipc_msg_free(rsp);

                break;

            case '1':
                sdk_ipc_reg_notify(MA, MSG_NTF);
                break;

            case '2':
                sdk_ipc_dereg_notify(MA, MSG_NTF);
                break;

            case '3':
                run_flag = 0;
                pthread_exit(0);
                break;

            default:
                break;
        }

        printf("==== 0-req, 1-reg_ntf, 2-dereg-ntf, 3-exit ====\n");
    }
}

int main()
{
    sdk_ipc_msg *msg = NULL, *rsp = NULL;
    int ret = -1;

    sdk_ipc_init(MB);

    pthread_create(&thread_pid, NULL, thread_routine, NULL);

    run_flag = 1;
    while(run_flag)
    {
        printf("waiting msg...\n");
        msg = sdk_ipc_wait_msg_timeout(3000);
        if (NULL == msg)
            continue;

        print_msg(msg);

        switch (msg->hdr.id)
        {
            case MSG_REQ:
                rsp = sdk_ipc_msg_alloc(sizeof(struct msg_data));
                rsp->hdr.id = MSG_RSP;
                strcpy(rsp->data, "reponse from mb");
                sdk_ipc_response(msg, rsp);
                sdk_ipc_msg_free(rsp);

                break;

            case MSG_RSP:
                break;

            case MSG_NTF:
                break;

            default:
                break;
        }

        sdk_ipc_msg_free(msg);
    }

    pthread_join(thread_pid, NULL);

    sdk_ipc_uninit();

    return 0;
}


