#include <unistd.h>

#include "sdk_timer.h"

void timer_cb(void *args)
{
    printf("timer timeout, %d\n", (int)args);
}

int main()
{
    sdk_timer *timer = NULL;
    struct sdk_timer_attr attr;
    int i = 0;

    sdk_ret sret = SDK_EINNER;

    sret = sdk_timer_init();

    for ( i = 0; i < 5; i++)
    {
        timer = sdk_timer_create();

        attr.timeout = 1000;
        attr.cycle = 1;
        attr.cb_func = timer_cb;
        attr.cb_data = i;
        sdk_timer_set_attr(timer, &attr);

        printf("start\n");
        sdk_timer_start(timer);
    }

    sleep(3);

    printf("restart\n");
    sdk_timer_restart(timer);

    sleep(3);

    printf("stop");
    sdk_timer_stop(timer);

    
    printf("restart\n");
    sdk_timer_restart(timer);

    sleep(3);
    
    printf("destroy\n");
    sdk_timer_destroy(timer);

    sret = sdk_timer_uninit();

    return 0;
}
