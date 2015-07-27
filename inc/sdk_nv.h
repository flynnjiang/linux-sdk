#ifndef __SDK_NV_H__
#define __SDK_NV_H__


#include "sdk_types.h"


/* NV Flags */
#define SDK_NV_FLAGS_NOT_EXPORT         1
#define SDK_NV_FLAGS_NOT_UPGRADE        (1 << 1)
#define SDK_NV_FLAGS_NOT_RESET          (1 << 2)

/* NV Storage Path */
#define SDK_NV_DIR             "/data/nv/"
#define SDK_NV_TMP_DIR         "/tmp/nv/"
#define SDK_NV_FACTORY_DIR     "/misc/nv/"

#define SDK_NV_DEFAULT_FILE     "/data/etc/default.nv"

#define SDK_NV_INIT_FLAG_FILE   SDK_NV_DIR"__init__"

sdk_ret sdk_nv_init();
sdk_ret sdk_nv_uninit();

sdk_ret sdk_nv_read(const char *name, char *buf, size_t buf_len);
sdk_ret sdk_nv_write(const char *name, const char *value);

sdk_ret sdk_nv_read_tmp(const char *name, char *buf, size_t buf_len);
sdk_ret sdk_nv_write_tmp(const char *name, const char *value);

sdk_ret sdk_nv_read_factory(const char *name, char *buf, size_t buf_len);
sdk_ret sdk_nv_write_factory(const char *name, const char *value);

sdk_ret sdk_nv_import(const char *fpath);
sdk_ret sdk_nv_export(const char *fpath);

sdk_ret sdk_nv_get_flags(const char *name, unsigned int *flags);


#endif /* __SDK_NV_H__ */
