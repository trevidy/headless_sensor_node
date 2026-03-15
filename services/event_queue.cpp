#include "event_queue.h"

static event_t queue[EVENT_QUEUE_SIZE];

static int head = 0;
static int tail = 0;

void event_queue_init()
{
    head = 0;
    tail = 0;
}

bool event_post(event_t event)
{
    int next = (head + 1) % EVENT_QUEUE_SIZE;

    if (next == tail)
    {
        return false; // queue full
    }

    queue[head] = event;
    head = next;

    return true;
}

bool event_get(event_t *event)
{
    if(head == tail)
    {
        return false; //queue empty
    }
    *event = queue[tail];
    tail = (tail + 1) % EVENT_QUEUE_SIZE;

    return true;
}