#pragma once

#include <stdint.h>

typedef enum
{
    LOG_INFO = 0,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

void logger_init();

void log_message(log_level_t level, const char *message);