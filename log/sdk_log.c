#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "sdk_sys.h"
#include "sdk_log.h"

static int g_log_level = SDK_LOG_LV_INFO;

sdk_ret print_console(const char *msg)
{
    int fd = -1;
    ssize_t len = 0;

    fd = open("/dev/console", O_WRONLY);
    if (fd < 0) {
        printf("open /dev/console failed, errno=%d\n", errno);
        return SDK_ESYS;
    }

    len = write(fd, msg, strlen(msg)+1);
    if (len != strlen(msg)+1) {
        printf("write log msg err, len = %d\n", len);
    }

    close(fd);

    return SDK_OK;
}

sdk_ret log_print(int level, const char *msg)
{
    if (NULL == msg) {
        printf("log_print: null pointer of msg\n");
        return SDK_EBADPARA;
    }

    if (level >= g_log_level) {
        //printf("%s\n", msg);
        print_console(msg);
    }

    return SDK_OK;
}


sdk_ret sdk_log_init()
{
    g_log_level = SDK_LOG_LV_INFO;

    return SDK_OK;
}

sdk_ret sdk_log_uninit()
{
    return SDK_OK;
}

sdk_ret sdk_log_print(int level, 
                      const char *tag, 
                      const char *file,
                      const char *func,
                      int line,
                      const char *fmt, ...)
{
    char buf[SDK_LOG_BUF_MAX_SIZE];
    int len = 0;

    char uptime[32];
    va_list args;

    memset(uptime, 0, sizeof(uptime));
    get_uptime(uptime, sizeof(uptime));

    /* Build log message */
    memset(buf, 0, sizeof(buf));
    len = snprintf(buf, sizeof(buf), "%s ", uptime);

    switch (level)
    {
        case SDK_LOG_LV_INFO:
            len += snprintf(buf+len, sizeof(buf)-len, "<I>");            
            break;
        case SDK_LOG_LV_WARN:
            len += snprintf(buf+len, sizeof(buf)-len, "<W>");            
            break;
        case SDK_LOG_LV_ERR:
            len += snprintf(buf+len, sizeof(buf)-len, "<E>");            
            break;
        default:
            len += snprintf(buf+len, sizeof(buf)-len, "<?>");            
            break;
    }

    len += snprintf(buf+len, sizeof(buf)-len, "[%s]", tag);
    //len += snprintf(buf+len, sizeof(buf)-len, "[%-10.10s:%3d:%10.10s] ", file, line, func);
    len += snprintf(buf+len, sizeof(buf)-len, "[%s:%d] ", file, line);

    va_start(args, fmt);
    len += vsnprintf(buf+len, sizeof(buf)-len, fmt, args);
    va_end(args);

    len += snprintf(buf+len, sizeof(buf)-len, "\n");

    /* Print log */
    log_print(level, buf);

    return SDK_OK;

}

