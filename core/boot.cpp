#include "boot.h"
#include "esp_system.h"
#include <stdio.h>

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
    
    printf("\n--- Reset Reason ---\n");
    printf("%s\n", reset_reason_to_string(reason));
    printf("----------------------\n");
}