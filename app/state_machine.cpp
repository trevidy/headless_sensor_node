#include "state_machine.h"
#include <stdio.h>
#include "logger.h"

static system_state_t current_state;

static void on_enter(system_state_t state);
static void on_exit(system_state_t state);

void state_machine_init()
{
    current_state = STATE_BOOT; //type enum 'system_state_t' can hold one value at a time.
    printf("state machine initialized\n");
}

system_state_t state_machine_get_state()
{
    return current_state;
}

static void transition_to(system_state_t new_state)
{
    if(new_state == current_state)
    {
        return;
    }    
    on_exit(current_state);
    
    //log_message(LOG_INFO, "State transition");

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

            if(event.type == EVT_TIMER_1S)
            {
                transition_to(STATE_IDLE);
            }

            break;

        case STATE_IDLE:

            if(event.type == EVT_SENSOR_TIMEOUT)
            {
                transition_to(STATE_FAULT);
            }

            break;

        case STATE_FAULT:

            if(event.type == EVT_TIMER_1S)
            {
                transition_to(STATE_IDLE);
            }

            break;

        case STATE_SAFE_MODE:
            //minimal behavior
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
            log_message(LOG_INFO, "Entering BOOT");
            break;
        
        case STATE_INIT:
            log_message(LOG_INFO, "Initializing system");
            break;

        case STATE_IDLE:
            log_message(LOG_INFO, "System idle");
            break;

        case STATE_ACTIVE:
            log_message(LOG_INFO, "System active");
            break;

        case STATE_FAULT:
            log_message(LOG_ERROR, "Fault detected");
            break;

        case STATE_SAFE_MODE:
            log_message(LOG_ERROR, "SAFE MODE");
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