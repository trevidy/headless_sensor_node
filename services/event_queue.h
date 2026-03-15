#pragma once

#include <stdint.h>
#define EVENT_QUEUE_SIZE 32

typedef enum
{
    EVT_NONE = 0,

    EVT_BOOT,
    EVT_TIMER_1S,

    EVT_SENSOR_TIMEOUT,
    EVT_COMM_FAILURE,

    EVT_FAULT
} event_type_t;

typedef struct
{
    event_type_t type;
    uint32_t data;
} event_t;

void event_queue_init();

bool event_post(event_t event);

bool event_get(event_t *event);