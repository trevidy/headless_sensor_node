#include "logger.h"

#include <stdio.h>

static const char* level_to_string(log_level_t level)
{
    switch(level)
    {
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default: return "UNK";
    }
}

void logger_init()
{
    printf("Logger initalized\n");
}

void log_message(log_level_t level, const char *message)
{
    printf("[%s] %s\n", level_to_string(level), message);
}