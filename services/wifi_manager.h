#pragma once
#include <stdbool.h>

void wifi_manager_init(const char *ssid, const char *password);
bool wifi_manager_is_connected();