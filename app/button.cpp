#include "button.h"
#include "driver/gpio.h"
#include "logger.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// can change the pin number
#define BUTTON_GPIO GPIO_NUM_4 

// handle for the task the ISR will notify. Must exist before the interrupt is registered, or an early press will try to notify a null handle
static TaskHandle_t button_task_handle = NULL;

// volatile since written from ISR context and read from task context
static volatile uint32_t isr_count = 0;

static void IRAM_ATTR button_isr_handler(void *arg){
    // esp_timer_get_time() returns microseconds since boot as int64_t. Truncation here is fine for now.
    uint32_t timestamp = (uint32_t)esp_timer_get_time();

    BaseType_t higher_priority_task_woken = pdFALSE;

    // send 32-bit value (timestamp) directly to the task identified by button_task_handle 
    xTaskNotifyFromISR(button_task_handle, // sitting blocked, ISR changes this state from Blocked->ready
                        timestamp,
                        eSetValueWithOverwrite, //new press overwrites any previous unread (mid-read) values
                        &higher_priority_task_woken);

    // if this notification just woke a higher-priority task than what was currently running,
    // do a context switch immediately instead of waiting for the next tick.
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

// dedicated task - spends almost all its time blocked until 
// the ISR wakes it
static void button_task(void *arg){
    uint32_t press_timestamp;

    while (true){
        // portMAX_DELAY = block forever until notified. 
        xTaskNotifyWait(0,0,&press_timestamp,portMAX_DELAY);

        printf("button_task woke - press timestamp (truncated us): %lu\n", (unsigned long)press_timestamp);
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
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&io_conf);

    // 3) install the shared GPIO ISR service
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        log_message(LOG_ERROR, "button: failed to install GPIO ISR service");
        return;
    }

    // 4) safe to now register the interrupt
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
    
    log_message(LOG_INFO, "button: ISR notifies button_task");
}
