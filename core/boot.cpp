#include "boot.h"
#include "esp_system.h"
#include <stdio.h>
#include "logger.h"
#include "config_store.h"
#include <stdint.h>

#include "nvs_flash.h"
#include "nvs.h"

#define NVS_NAMESPACE "boot"
#define NVS_KEY_FAIL "fail_count"

static int load_fail_count() {
    nvs_handle_t h;
    int32_t count = 0;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY,&h) == ESP_OK){
        nvs_get_i32(h, NVS_KEY_FAIL, &count);
        nvs_close(h);
    }
    return (int)count;
}

static void save_fail_count(int count){
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) == ESP_OK){
        nvs_set_i32(h, NVS_KEY_FAIL, (int32_t)count);
        nvs_commit(h);
        nvs_close(h);
    }
}

// config_block_t cfg = *config_get();

static const char* reset_reason_to_string(esp_reset_reason_t reason)
{
    switch(reason)
    {
        case ESP_RST_POWERON: return "Power-on";
        case ESP_RST_EXT: return "External reset";
        case ESP_RST_SW: return "Software reset";
        case ESP_RST_PANIC: return "Kernel panic";
        case ESP_RST_INT_WDT: return "Interrupt watchdog";
        case ESP_RST_TASK_WDT: return "Task watchdog";
        case ESP_RST_WDT: return "Other watchdog";
        case ESP_RST_DEEPSLEEP: return "Wake from deep sleep";
        case ESP_RST_BROWNOUT: return "Brownout";
        default: return "Unknown";
    }
}

void boot_log_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();
    int boot_fail_count = load_fail_count();

    // printf("Reset reason: %s\n", reset_reason_to_string(reason));

    char reset_reason_buf[100];
    snprintf(reset_reason_buf, sizeof(reset_reason_buf), "Reset Reason: %s", reset_reason_to_string(reason));
    log_message(LOG_INFO, reset_reason_buf);

    char boot_fail_before_buf[32];
    snprintf(boot_fail_before_buf, sizeof(boot_fail_before_buf), "Boot fail count BEFORE: %d", boot_fail_count);
    log_message(LOG_INFO, boot_fail_before_buf);
    // printf("Boot fail count BEFORE: %d\n", boot_fail_count);
 
    if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_PANIC)
    {
        boot_fail_count++;
        save_fail_count(boot_fail_count);
    }
    else if (reason == ESP_RST_POWERON)
    {
        boot_fail_count = 0;
        save_fail_count(boot_fail_count);
    }

    if (boot_fail_count>=3)
    {
        log_message(LOG_ERROR, "Entering SAFE MODE");
        // trigger safe mode event or flag
    }

    //printf("boot fail count AFTER: %d\n", boot_fail_count);
    //printf("boot fail count AFTER: %d\n", cfg.boot_fail_count);

    // log_message(LOG_INFO,"\n--- Reset Reason ---\n");
    //log_message(LOG_INFO, reset_reason_to_string(reason));
   
    char boot_fail_after_buf[32];
    snprintf(boot_fail_after_buf, sizeof(boot_fail_after_buf), "Boot fail count AFTER: %d", boot_fail_count);
    log_message(LOG_INFO, boot_fail_after_buf);
    log_message(LOG_INFO,"----------------------\n");
}