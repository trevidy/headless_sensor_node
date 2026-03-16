#include "state_machine.h"
#include <stdio.h>

static system_state_t current_state;

static void on_enter(system_state_t state);
static void on_exit(system_state_t state);

void state_machine_init()
{
    current_state = STATE_BOOT; //type enum 'system_state_t' can hold one value at a time.
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
    
    printf("STATE: %d -> %d\n", current_state, new_state);

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
            printf("Entering BOOT\n");
            break;
        
        case STATE_INIT:
            printf("Initializing system\n");
            break;

        case STATE_IDLE:
            printf("System idle\n");
            break;

        case STATE_ACTIVE:
            printf("System active\n");
            break;

        case STATE_FAULT:
            printf("Fault detected\n");
            break;

        case STATE_SAFE_MODE:
            printf("SAFE MODE\n");
            break;
    }
}

static void on_exit(system_state_t state)
{
    switch (state)
    {
        case STATE_ACTIVE:
            printf("Stopping active operations\n");
            break;
        
        default:
            break;
    }
}