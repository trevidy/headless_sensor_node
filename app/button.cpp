#include "button.h"
#include "driver/gpio.h"
#include "logger.h"
#include "esp_attr.h"

// can change the pin number
#define BUTTON_GPIO GPIO_NUM_4 

// volatile since written from ISR context and read from task context
static volatile uint32_t isr_count = 0;

static void IRAM_ATTR button_isr_handler(void *arg){
    isr_count++;
}

void button_init(){
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // fires on press
    gpio_config(&io_conf);

    // installs the shared GPIO ISR service
    esp_err_t err = gpio_install_isr_service(0);
    
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        log_message(LOG_ERROR, "button: failed to install GPIO ISR service");
        return;
    }

    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    log_message(LOG_INFO, "button: ISR registered on GPIO4");

}

uint32_t button_get_isr_count(){
    return isr_count;
}