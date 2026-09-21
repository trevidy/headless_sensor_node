#include "button.h"
#include "driver/gpio.h"
#include "logger.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "event_queue.h"

// can change the pin number
#define BUTTON_GPIO GPIO_NUM_4 

// ignore any edge within this window: 50ms
#define DEBOUNCE_US 50000

// hold duration threshold for a long press
#define LONG_PRESS_MS 600

// handle for the task the ISR will notify. Must exist before the interrupt is registered, or an early press will try to notify a null handle
static TaskHandle_t button_task_handle = NULL;

// ISR now fires on both edges (press and release), since we're measuring hold duration
// This does the minimum possibe: wake the task. 
// The woken task should read the gpio_get_level() once running
static void IRAM_ATTR button_isr_handler(void *arg){
    BaseType_t higher_priority_task_woken = pdFALSE;

    // lightweight binary/counting signal
    vTaskNotifyGiveFromISR(button_task_handle, &higher_priority_task_woken);

    // if this notification just woke a higher-priority task than what was currently running,
    // do a context switch immediately instead of waiting for the next tick.
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

// dedicated task - spends almost all its time blocked until 
// the ISR wakes it
static void button_task(void *arg){
    // last_edge_time: debounce reference point
    // press_start_time: only set when register an actual press
    // currently_pressed: current understanding of the button state, independent of raw GPIO noise
    static int64_t last_edge_time = 0;
    static int64_t press_start_time = 0;
    static bool currently_pressed = false;

    while (true){
        // task waits in a blocked state until its notification value becomes greater than zero.
        // pdTRUE: Binary semaphore - clears the notification count back to 0 after reading
        // pdFALSE: Counting semaphore - internal notification counter increments to how ever many times it is notified.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        int64_t now = esp_timer_get_time(); // full precision, no truncation

        if (now - last_edge_time < DEBOUNCE_US){
            continue; //too soon after the last edge. This is bounce, so continue to the next iteration of the while loop.
        }
        last_edge_time = now;

        int level = gpio_get_level(BUTTON_GPIO); // 0 = PRESSED (pulled to GND), 1 = released

        if (level == 0 && !currently_pressed){
            // genuine press start
            currently_pressed = true;
            press_start_time = now;
        }

        else if (level == 1 && currently_pressed){
            // genuine release - now we know the full hold duration
            currently_pressed = false;
            int64_t duration_ms = (now - press_start_time) / 1000;

            if (duration_ms >= LONG_PRESS_MS){
                printf("LONG PRESS (%lld ms)\n", (long long)duration_ms);
                event_post({EVT_BUTTON_LONG_PRESS, 0});
            }
            else{
                printf("SHORT PRESS (%lld ms)\n", (long long)duration_ms);
                event_post({EVT_BUTTON_SHORT_PRESS, 0});
            }
        
        }
        
    }
}

void button_init(){
    
    // 1) create the task FIRST so the handle is valid before any interrupt can possibly fire
    xTaskCreate(button_task, "button_task", 2048, NULL, 2, &button_task_handle);

    // 2) configure the GPIO
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&io_conf);

    // 3) install the shared GPIO ISR service
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        log_message(LOG_ERROR, "button: failed to install GPIO ISR service");
        return;
    }

    // 4) safe to now register the interrupt
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
    
    log_message(LOG_INFO, "button: debounce + short/long press detection");
}
