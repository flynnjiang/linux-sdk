
#include <sys/stat.h>
#include <sys/types.h>

#include "sdk_nv.h"



int main()
{
    char buf[32] = {0};
    sdk_ret sret = SDK_EINNER;

    printf(SDK_NV_INIT_FLAG_FILE);

    //mkdir(SDK_NV_DIR, 0755);

#if 1
    sret = sdk_nv_init();
    if (SDK_OK != sret)
        printf("sret = %d\n", sret);

    sdk_nv_write("nv_name", "nv_value");
    sdk_nv_read("nv_name", buf, sizeof(buf));

    printf("read nv val = %s\n", buf);

    sdk_nv_uninit();
#endif

    return 0;
}
