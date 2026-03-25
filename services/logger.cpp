#include "logger.h"
#include "esp_log.h"
#include <stdio.h>
#define TAG "APP"

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
    //ESP_LOGE : error
    //ESP_LOGW : warning
    //ESP_LOGI : info
    //ESP_LOGD : debug
    //ESP_LOGV : verbose
    ESP_LOGI(TAG, "%s", message);
}