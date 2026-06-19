#include "runtime.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "event_queue.h"
#include "state_machine.h"
#include "logger.h"
#include "config_store.h"
#include "watchdog.h"
#include "nvs_flash.h"
#include "boot.h"
#include "sensor.h"
#include "crash_log.h"
#include "wifi_manager.h"

#include "esp_heap_caps.h" // head monitoring
#include "esp_timer.h" // uptime tracking

void log_memory_usage(){
    size_t free = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    printf("Free heap: %d bytes\n", free);
}

void runtime_start()
{
    printf("Runtime starting...\n");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    logger_init();
    crash_log_init();
    config_init();
    watchdog_init();
    event_queue_init();
    state_machine_init();
    sensor_init(); 
    wifi_manager_init("TELUS0605", "FK6xnrG7hkVh26nX");

    event_post({EVT_BOOT,0}); 
    event_post({EVT_INIT_DONE,0}); // signal INIT complete

    event_t event;

    while (true)
    {
        static int counter = 0;
        counter++;
        
        if (counter >=100) // loop every 1 second 
        {
            counter = 0;

            sensor_reading_t reading;
            if (sensor_read(&reading))
            {
                printf("Temp: %.1fC Humidity: %.1f %% Pressure: %.1f hPa\n", reading.temperature, reading.humidity, reading.pressure);
                event_post({EVT_SENSOR_READY,0});
            }
            else{
                printf("Did not get sensor reading..\n.");
                event_post({EVT_SENSOR_TIMEOUT,0});
            }

            log_memory_usage();
            int64_t uptime_sec = esp_timer_get_time()/1000000;
            printf("Uptime: %lld s\n", uptime_sec);
            boot_count();
        }
        
        if (event_get(&event))
        {
            state_machine_handle_event(event);
        }

        if (state_machine_should_kick_wdt())
        {
            watchdog_kick(); //twdt for IDLE, INIT, ACTIVE, & BOOT. Not for FAULT & SAFE_MODE
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}