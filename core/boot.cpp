#include "boot.h"
#include "esp_system.h"
#include <stdio.h>
#include "logger.h"
#include "config_store.h"
#include <stdint.h>
#include "driver/rtc_io.h"
#include "esp_attr.h"

RTC_NOINIT_ATTR uint32_t boot_fail_count;
RTC_NOINIT_ATTR uint32_t boot_magic;

#define BOOT_MAGIC 0xDEADBEEF

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
    printf("called\n");
    esp_reset_reason_t reason = esp_reset_reason();
    
    printf("Reset reason: %s\n", reset_reason_to_string(reason));

    // char reset_reason_buf[100];
    // snprintf(reset_reason_buf, sizeof(reset_reason_buf), "Reset Reason: %s", reset_reason_to_string(reason));
    // log_message(LOG_INFO, reset_reason_buf);

    // char boot_fail_before_buf[32];
    // snprintf(boot_fail_before_buf, sizeof(boot_fail_before_buf), "Boot fail count BEFORE: %d", boot_fail_count);
    // log_message(LOG_INFO, boot_fail_before_buf);
    // printf("Boot fail count BEFORE: %d\n", boot_fail_count);
 
    if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_PANIC)
    {

        if (boot_magic != BOOT_MAGIC) {
            boot_magic = BOOT_MAGIC;
            boot_fail_count = 0;
        }
        printf("boot fail count BEFORE: %ld\n", boot_fail_count);
        boot_fail_count++;

        printf("boot fail AFTER: %ld\n", boot_fail_count);

        if (boot_fail_count >= 3) {
            // Enter safe mode, erase NVS, notify user, etc.
            printf("ENTERING SAFE MODE\n");
    }

    }
    else if (reason == ESP_RST_POWERON)
    {
        if (boot_magic != BOOT_MAGIC) {
            boot_magic = BOOT_MAGIC;
            boot_fail_count = 0;
            printf("Device powered on safely. Boot fail count is: %ld\n", boot_fail_count);
        }
    }

   
    // char boot_fail_after_buf[32];
    // snprintf(boot_fail_after_buf, sizeof(boot_fail_after_buf), "Boot fail count AFTER: %d", boot_fail_count);
    // log_message(LOG_INFO, boot_fail_after_buf);
    // log_message(LOG_INFO,"----------------------\n");
}