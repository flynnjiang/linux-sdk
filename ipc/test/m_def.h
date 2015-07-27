

enum {
    MA = 1,
    MB
};


enum {
    MSG_REQ = 1,
    MSG_RSP,
    MSG_NTF
};

struct msg_data {
    char str[32];
};


static inline void print_msg(sdk_ipc_msg *msg)
{
    struct msg_data *mdata = NULL;

    mdata = msg->data;
    printf("type=%d, id=%d, src=%d, dest=%d, sid=%d, datalen=%d, data[0]=%c\n",
            msg->hdr.type, msg->hdr.id, msg->hdr.src,
            msg->hdr.dest, msg->hdr.sid, msg->hdr.data_len, *((char *)(msg->data)));
}
