#include "config_store.h"
#include "esp_system.h"
#include "esp_crc.h"

#include <string.h>

#include "nvs_flash.h"
#include "nvs.h"
#include "logger.h"

static config_block_t current_config;

static bool flash_write_stub(const config_block_t* cfg)
{
    // Simulate successful write
    return true;
}

static bool flash_read_stub(config_block_t* cfg)
{
    *cfg = current_config;
    return true;
}


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

static bool verify_config(const config_block_t *cfg)
{
    uint32_t computed_crc = compute_crc((config_block_t*)cfg);

    return (computed_crc == cfg->crc) && (cfg->version == CONFIG_VERSION);
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
    
    // simulated flash write
    bool write_ok = flash_write_stub(cfg);

    config_block_t readback;
    bool read_ok = flash_read_stub(&readback);

    // simulated verification
    bool verify_success = write_ok && read_ok;

    // verify crc
    if (verify_config(&readback) && verify_success)
    {
        current_config = readback; // take all the data from the pointer cfg (temporary workspace) and overwrite the main global variable current_config
        return true;
    }
    
    return false;
}