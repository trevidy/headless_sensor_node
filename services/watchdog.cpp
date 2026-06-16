#include "watchdog.h"

#include "esp_task_wdt.h"
#include "logger.h"

#define WATCHDOG_TIMEOUT_SEC 10

void watchdog_init()
{
    // allocate and populate the configuration structure for the Task Watchdog Timer (TWDT)
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT_SEC * 1000, // convert seconds to milliseconds (e.g., 10 seconds * 1000 = 10000ms)
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // bitmask to select which cpu cores to watch. (1<<2) - 1 evaluates to 3 (binary 011), which enables watching the Idle task on both core 0 and core 2. ESP32 has two cores.
        .trigger_panic = true, // if a task starves the watchdog, immediately halt the system and trigger a hardware reset/reboot
    };

    esp_err_t err = esp_task_wdt_init(&twdt_config); // pass the configuration structure to the ESP-IDF driver to initialize or reconfigure the watchdog hardware.

    if (err == ESP_ERR_INVALID_STATE)
    {
        // the framework is already initialized at boot
        log_message(LOG_INFO, "Watchdog already initialized by system. Skipping custom init.");
        esp_task_wdt_add(NULL);
        return;
    } 

    else if (err != ESP_OK){
        log_message(LOG_ERROR, "watchdog initialization failed with severe error");
        return;
    }

    // this section only executes if the system HADN'T initialiezd yet
    esp_task_wdt_add(NULL); //esp_task_wdt_add([subscribe to a specific task]), so subscribe to the main task
    log_message(LOG_INFO,"Watchdog initialized");
}

void watchdog_kick()
{
    esp_task_wdt_reset();
}
