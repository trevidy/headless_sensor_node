#include <stdio.h>

#include "boot.h"
#include "runtime.h"

extern "C" void app_main(void)
{
    printf("\n");
    printf("============================\n");
    printf(" Headless Sensor node v1\n");
    printf(" Booting...\n");
    printf("============================\n");

    boot_log_reset_reason();

    while (true)
    {
        runtime_start();
    }
}