#include "config_store.h"
#include "esp_system.h"
#include "esp_crc.h"

#include <string.h>

static config_block_t current_config;

static uint32_t compute_crc(config_block_t *cfg)
{
    //esp_crc32_le() is a function from the ESP-IDF which calculates corruption. 
    // argument 1: 0
    // argument 2: starting memory address of the data you want to check
    // argument 3: total number of bytes to process.
    return esp_crc32_le(0,
                        (uint8_t*)cfg +12,
                        sizeof(config_block_t)-12);
}

static void load_defaults()
{
    //memset(): fill a block of memory with a specific value (usually zero)
    // argument 1: the destination
    // argument 2: the value
    // argument 3: the size
    memset(&current_config,0,sizeof(current_config));

    current_config.version = CONFIG_VERSION;
    current_config.seq = 1;

    current_config.sample_interval_ms = 1000;
    current_config.device_id = 1;
}

void config_init()
{
    load_defaults();

    current_config.crc = compute_crc(&current_config);
}

const config_block_t* config_get()
{
    return &current_config;
}

bool config_save(config_block_t *cfg)
{
    cfg->seq++;
    cfg->crc = compute_crc(cfg); //before saving, run a mathematical formula over all the data in the struct. result is stored in crc and will be compared with a re-calculated crc value when the device reboots. if they dont match, the code knows the memory was corrupted.
    current_config = *cfg; // take all the data from the pointer cfg (temporary workspace) and overwrite the main global variable current_config

    return true;
}