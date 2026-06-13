#pragma once
#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    float pressure;
} sensor_reading_t;

void sensor_init();
bool sensor_read(sensor_reading_t *out);  // true = success, false = timeout/fault