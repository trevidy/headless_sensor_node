#include <stdio.h>

#include "boot.h"
#include "runtime.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern "C" void app_main(void)
{
    printf("\n");
    printf("BUILD: %s %s\n", __DATE__, __TIME__);
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