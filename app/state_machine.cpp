#include "state_machine.h"
#include "freertos/FreeRTOS.h"  // Added for pdMS_TO_TICKS
#include "freertos/task.h"      // Added for vTaskDelay
#include "watchdog.h"
#include "logger.h"
#include "esp_system.h"
#include <stdio.h>

static system_state_t current_state;
static int fault_count = 0;

#define MAX_FAULTS 3

static void on_enter(system_state_t state);
static void on_exit(system_state_t state);

void state_machine_init()
{
    current_state = STATE_BOOT; //type enum 'system_state_t' can hold one value at a time.
    fault_count = 0;
    log_message(LOG_INFO, "state machine initialized\n");
    on_enter(STATE_BOOT); //manually enter since we bypass transition_to()
}

system_state_t state_machine_get_state()
{
    return current_state;
}

// Call this from runtime loop - true means kick the watchdog, false means don't
bool state_machine_should_kick_wdt()
{
    return (current_state == STATE_IDLE   || 
            current_state == STATE_INIT   ||
            current_state == STATE_ACTIVE ||
            current_state == STATE_BOOT);
}


static void transition_to(system_state_t new_state)
{
    if(new_state == current_state)
    {
        return;
    }    
    on_exit(current_state);

    current_state = new_state;

    on_enter(new_state);
}

void state_machine_handle_event(event_t event)
{
    switch (current_state)
    {
        case STATE_BOOT:

            if(event.type == EVT_BOOT)
            {
                transition_to(STATE_INIT);
            }
            
            break;
        
        case STATE_INIT:

            if(event.type == EVT_INIT_DONE)
            {
                transition_to(STATE_IDLE);
            }

            break;

        case STATE_IDLE:

            if (event.type == EVT_SENSOR_READY)
            {
                transition_to(STATE_ACTIVE);
            }

            else if(event.type == EVT_SENSOR_TIMEOUT)
            {
                transition_to(STATE_FAULT);
            }

            break;

        case STATE_ACTIVE:

            if(event.type == EVT_PROCESS_DONE)
            {
                transition_to(STATE_IDLE);
            }
            
            else if (event.type == EVT_SENSOR_TIMEOUT)
            {
                transition_to(STATE_FAULT);
            }

            break;
            
        case STATE_FAULT:

            if(event.type == EVT_FAULT_CLEARED)
            {
                transition_to(STATE_IDLE);
            }
            
            else if (event.type == EVT_TIMER_1S)
            {
                fault_count++;
                if (fault_count >= MAX_FAULTS)
                    transition_to(STATE_SAFE_MODE);
                else
                    transition_to(STATE_IDLE);
            }

            break;

        case STATE_SAFE_MODE:
            //wdt will fire and reboot us
            break;
        
        default:
            break;
    }
}

static void on_enter(system_state_t state)
{
    switch (state)
    {
        case STATE_BOOT:
            log_message(LOG_INFO, "=== BOOT ===");
            break;
        
        case STATE_INIT:
            log_message(LOG_INFO, "=== INIT ===");
            break;

        case STATE_IDLE:
            log_message(LOG_INFO, "=== IDLE ===");
            break;

        case STATE_ACTIVE:
            log_message(LOG_INFO, "=== ACTIVE ===");
            event_post({EVT_PROCESS_DONE,0});
            break;

        case STATE_FAULT:
            log_message(LOG_ERROR, "=== FAULT === recoverable error detected ===");
            break;

        case STATE_SAFE_MODE:
            log_message(LOG_FATAL, "=== SAFE MODE === too many faults, rebooting in 5s");
            // don't kick the WDT, but give the logger a moment to flush
            vTaskDelay(pdMS_TO_TICKS(5000)); //only pauses this task. other tasks continues to run. 
            esp_restart();
            break;
    }
}

static void on_exit(system_state_t state)
{
    switch (state)
    {
        case STATE_ACTIVE:
            log_message(LOG_INFO, "Stopping active operations");
            break;
        
        default:
            break;
    }
}