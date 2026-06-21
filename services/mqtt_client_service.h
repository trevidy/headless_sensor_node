#pragma once
#include <stdbool.h>
#include "sensor.h"

void mqtt_service_init(const char *broker_uri);
void mqtt_publish_telemetry(const sensor_reading_t *reading, const char *state_name, int fault_count);
void mqtt_publish_fault(const char *reason);
bool mqtt_service_is_connected();