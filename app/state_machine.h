#pragma once
#include "event_queue.h"

typedef enum 
{
    //essentially just numbers, but we give names for readability

    STATE_BOOT = 0,   //system just started 
    STATE_INIT,       //initializing services
    STATE_IDLE,       //ready but not doing work
    STATE_ACTIVE,     //normal operation
    STATE_FAULT,      //recoverable error
    STATE_SAFE_MODE   //minimal functionality
} system_state_t;

void state_machine_init();

void state_machine_handle_event(event_t event);

system_state_t state_machine_get_state();