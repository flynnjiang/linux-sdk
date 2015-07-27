#ifndef __SDK_SYS_H__
#define __SDK_SYS_H__


#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "sdk_types.h"


static inline int file_lock(int fd, int cmd, 
                  int type, off_t start,
                  int whence, off_t len)
{
    struct flock lock;

    lock.l_type = type;
    lock.l_start = start;
    lock.l_whence = whence;
    lock.l_len = len;

    return fcntl(fd, cmd, &lock);
}

/* File read lock */
#define read_lock(fd)   \
    file_lock(fd, F_SETLKW, F_RDLCK, 0, SEEK_SET, 0)

/* File write lock */
#define write_lock(fd)   \
    file_lock(fd, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0)

/* Unlock file */
#define unlock(fd)   \
    file_lock(fd, F_SETLKW, F_UNLCK, 0, SEEK_SET, 0)


/* Get system uptime */
static inline sdk_ret get_uptime(char *buf, size_t buf_len)
{
    FILE *fp = NULL;
    char uptime[32] = {0};

    if (NULL == buf)
        return SDK_EBADPARA;

    fp = fopen("/proc/uptime", "r");
    if (NULL == fp)
        return SDK_ESYS;

    fscanf(fp, "%s %*s", uptime);
    fclose(fp);

    memset(buf, 0, buf_len);
    strncpy(buf, uptime, buf_len - 1);

    return SDK_OK;
}

#endif /* __SDK_SYS_H__ */
