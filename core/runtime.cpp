#include "runtime.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void runtime_start()
{
    printf("Runtime starting...\n");

    while (true)
    {
        //future code implementation

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}