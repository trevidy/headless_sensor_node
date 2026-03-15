#include "runtime.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "event_queue.h"

void runtime_start()
{
    printf("Runtime starting...\n");

    event_queue_init();

    // First event
    event_post({EVT_BOOT,0});

    event_t event;

    while (true)
    {
        static int counter = 0;
        counter++;

        if(counter==100)
        {
            event_post({EVT_TIMER_1S,0});
            counter = 0;
        }
        
        if(event_get(&event))
        {
            printf("Event received: %d\n", event.type);
        }

        

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}