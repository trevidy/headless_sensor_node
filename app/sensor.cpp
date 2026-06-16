#include "sensor.h"
#include "logger.h"
#include "bmp280.h"
#include <cstring>
#include <stdio.h>
#include "i2cdev.h"

/*
bmp280t is a struct defined by the driver that holds everything about the sensor.
values:
dig_T1 to dig_T3: mathematical constraints used to calibrate the temperature readings
dig_P1 to dig_P9: mathematical constraints used to calibrate the barometric pressure readings
dig_H1 to dig_H6: mathematical constraints used to calibrate the humidity readings. 
i2c_dev: I2C device descriptor, a nested structure (from the esp-idf-lib I2C driver) that holds the physical connection details.
 - tells which I2C port is being used (I2C_NUM_0)
 - which ESP32 pins are acting as SDA and SCL
 - the specific I2C address of the sensor (usually 0x76 or 0x77)
*/

static bmp280_t dev; 
static bool initialized = false;

void sensor_init()
{
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    params.mode = BMP280_MODE_NORMAL;

    memset(&dev, 0 , sizeof(dev)); //zero out the entire dev struct for safety before writing.
    dev.i2c_dev.port = I2C_NUM_0; //set I2C bus
    dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0; //0x76 via structural field
    dev.i2c_dev.cfg.sda_io_num = GPIO_NUM_21;
    dev.i2c_dev.cfg.scl_io_num = GPIO_NUM_22;
    dev.i2c_dev.cfg.master.clk_speed = 400000;  // 400kHz fast mode

    i2cdev_init();  // initializes the global I2C mutex system
    esp_err_t err = bmp280_init(&dev, &params);     
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

