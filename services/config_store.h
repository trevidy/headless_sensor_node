#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_VERSION 1

typedef struct
{
    //meta data. skip this in crc calulation.
    //uint32_t: each takes 4 bytes in memory.
    uint32_t version;
    uint32_t seq;
    uint32_t crc;

    uint32_t sample_interval_ms;
    uint32_t device_id;
} config_block_t;

void config_init();

const config_block_t* config_get();

bool config_save(config_block_t *cfg);