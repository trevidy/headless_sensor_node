#include <stdio.h>

#include "boot.h"
#include "runtime.h"
#include "nvs_flash.h"
#include "nvs.h"

extern "C" void app_main(void)
{
    printf("\n");
    printf("============================\n");
    printf(" Headless Sensor node v1\n");
    printf(" Booting...\n");
    printf("============================\n");
    
    //initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { //check for full storage or new software
        ESP_ERROR_CHECK(nvs_flash_erase()); // wipe clean the NVS patition.
        ret = nvs_flash_init(); //re-initialize the nvs
    }
    ESP_ERROR_CHECK(ret);

    boot_log_reset_reason();

    while (true)
    {
        runtime_start();
    }
}