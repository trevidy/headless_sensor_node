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

#include "esp_heap_caps.h" // head monitoring
#include "esp_timer.h" // uptime tracking

void log_memory_usage(){
    size_t free = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    printf("Free heap: %d bytes\n", free);
}

void runtime_start()
{
    printf("Runtime starting...\n");

    logger_init();
    config_init();
    watchdog_init();

    event_queue_init();
    state_machine_init();

    // First event
    event_post({EVT_BOOT,0});

    event_t event;

    while (true)
    {
        static int counter = 0;
        counter++;
        
        if (counter >=100)
        {
            event_post({EVT_TIMER_1S,0});
            counter = 0;

            log_memory_usage();
            int64_t uptime_sec = esp_timer_get_time()/1000000;
            printf("Uptime: %lld s\n", uptime_sec);
            boot_count();
            printf("\n");
        }
        

        if (event_get(&event))
        {
            state_machine_handle_event(event);
        }

        watchdog_kick();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}