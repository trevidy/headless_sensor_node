#include "sensor.h"
#include "logger.h"
#include "bmp280.h"
#include <stdio.h>

static bmp280_t dev;
static bool initialized = false;

void sensor_init()
{
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    params.mode = BMP280_MODE_NORMAL;

    memset(&dev, 0 , sizeof(dev));

    esp_err_t err = bmp280_init(&dev, &params, 0x76); //0x76 is default I2C addr
    if (err != ESP_OK){
        log_message(LOG_ERROR, "BME280 init failed");
        initialized = false;
        return;
    }
    
    initialized = true;
    log_message(LOG_INFO, "BME280 initialized");

}

bool sensor_read(sensor_reading_t *out)
{
    if (!initialized)
    {
        return false;
    }

    float pressure, temp, humidity;
    esp_err_t err = bmp280_read_float(&dev, &temp, &pressure, &humidity);
    
    if (err != ESP_OK){
        log_message(LOG_ERROR, "BME280 read failed");
        return false;
    }

    out->temperature = temp;
    out->humidity = humidity;
    out->pressure = pressure / 100.0f; // Pa -> hPa

    return true;
}

