#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "sdk_ipc.h"
#include "m_def.h"



int main()
{
    sdk_ipc_msg *msg = NULL, *rsp = NULL, *ntf = NULL;
    int ret = -1;

    sdk_ipc_init(MA);

    while(1)
    {
        printf("waiting msg...\n");
        msg = sdk_ipc_wait_msg();

        printf("got msg\n");
        print_msg(msg);

        switch (msg->hdr.id)
        {
            case MSG_REQ:
                /* Send response */
                rsp = sdk_ipc_msg_alloc(sizeof(struct msg_data));
                rsp->hdr.id = MSG_RSP;
                strcpy(rsp->data, "reponse from ma");
                sdk_ipc_response(msg, rsp);
                sdk_ipc_msg_free(rsp);

                /* boardcast notify */
                ntf = sdk_ipc_msg_alloc(sizeof(struct msg_data));
                ntf->hdr.id = MSG_NTF;
                strcpy(ntf->data, "notification from ma");
                sdk_ipc_notify_bdcast(ntf);
                sdk_ipc_msg_free(ntf);

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

    sdk_ipc_uninit();

    return 0;
}


