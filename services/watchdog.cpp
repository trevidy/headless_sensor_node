#include "watchdog.h"

#include "esp_task_wdt.h"
#include "logger.h"

#define WATCHDOG_TIMEOUT_SEC 10

void watchdog_init()
{
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT_SEC * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true,
    };
    esp_task_wdt_init(&twdt_config); 

    esp_task_wdt_add(NULL); //esp_task_wdt_add([subscribe to a specific task])

    log_message(LOG_INFO,"Watchdog initialized");
    }

void watchdog_kick()
{
    esp_task_wdt_reset();
}
