#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "sdk_sys.h"
#include "sdk_nv.h"


sdk_ret nv_import(const char *fpath, const char *dir, int mode)
{
    return SDK_OK;
}

sdk_ret nv_export(const char *fpath, const char *dir, int mode)
{
    return SDK_OK;
}


sdk_ret nv_read(const char *path, char *buf, size_t buf_len)
{
    int fd = -1;
    int ret = -1;

    if (NULL == path || NULL == buf)
        return SDK_EBADPARA;

    memset(buf, 0, buf_len);

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return SDK_ESYS;

    read_lock(fd);
    ret = read(fd, buf, buf_len);
    unlock(fd);
    close(fd);

    if (ret < 0)
        return SDK_ESYS;

    return SDK_OK;
}

sdk_ret nv_write(const char *path, const char *value)
{
    int fd = -1;
    int len = 0;
    int ret = -1;

    if (NULL == path || NULL == value)
        return SDK_EBADPARA;

    len = strlen(value) + 1;

    fd = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd < 0)
        return SDK_ESYS;

    write_lock(fd);
    ret = write(fd, value, len);
    ftruncate(fd, len);
    unlock(fd);
    close(fd);

    if (ret != len)
        return SDK_ESYS;

    return SDK_OK;
}

sdk_ret sdk_nv_init()
{
    int fd = -1;

    umask(0);

    /* NV has been init before */
    if (0 == access(SDK_NV_INIT_FLAG_FILE, F_OK))
        return SDK_OK;

    if (access(SDK_NV_DIR, F_OK) < 0)
    {
        if (mkdir(SDK_NV_DIR, 0775) < 0 && errno != EEXIST)
            return SDK_ESYS;
    }

    if (access(SDK_NV_DEFAULT_FILE, R_OK) == 0)
        nv_import(SDK_NV_DEFAULT_FILE, SDK_NV_DIR, 1);

    /* Create init flag file */
    fd = open(SDK_NV_INIT_FLAG_FILE, O_WRONLY | O_CREAT, 0666);
    if (fd < 0)
        return SDK_ESYS;
    close(fd);    
    
    return SDK_OK;
}

sdk_ret sdk_nv_uninit()
{
    /* Do Nothing. */

    return SDK_OK;
}

sdk_ret sdk_nv_read(const char *name, char *buf, size_t buf_len)
{
    char path[64] = {0};

    snprintf(path, sizeof(path), "%s%s", SDK_NV_DIR, name);

    return nv_read(path, buf, buf_len);
}

sdk_ret sdk_nv_write(const char *name, const char *value)
{
    char path[64] = {0};

    snprintf(path, sizeof(path), "%s%s", SDK_NV_DIR, name);

    return nv_write(path, value);
}

sdk_ret sdk_nv_read_tmp(const char *name, char *buf, size_t buf_len)
{
    char path[64] = {0};

    snprintf(path, sizeof(path), "%s%s", SDK_NV_TMP_DIR, name);

    return nv_read(path, buf, buf_len);
}

sdk_ret sdk_nv_write_tmp(const char *name, const char *value)
{
    char path[64] = {0};

    snprintf(path, sizeof(path), "%s%s", SDK_NV_TMP_DIR, name);

    return nv_write(path, value);
}

sdk_ret sdk_nv_read_factory(const char *name, char *buf, size_t buf_len)
{
    return SDK_EINNER;
}

sdk_ret sdk_nv_write_factory(const char *name, const char *value)
{
    return SDK_EINNER;
}

sdk_ret sdk_nv_import(const char *fpath)
{
    return nv_import(fpath, SDK_NV_DIR, 0);
}

sdk_ret sdk_nv_export(const char *fpath)
{
    return nv_export(fpath, SDK_NV_DIR, 0);
}

sdk_ret sdk_nv_get_flags(const char *name, unsigned int *flags)
{
    return SDK_EINNER;
}
