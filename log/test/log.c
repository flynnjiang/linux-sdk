
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "sdk_log.h"

#define SDK_LOG_TAG "test"

int main()
{
    char buf[32] = {0};
    sdk_ret sret = SDK_EINNER;

    sdk_log_init();

    LOG_INFO("iiiiiii = %d", 1111);

    sleep(1);

    LOG_WARN("wwwwwwww = %d", 1111);

    sleep(1);

    LOG_ERR("eeeeeeee = %d", 1111);

    sdk_log_uninit();

    return 0;
}
