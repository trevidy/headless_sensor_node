#pragma once
#include <stdint.h>

typedef struct {
    uint32_t timestamp_sec;
    uint8_t reset_reason;
    uint8_t fault_count;
    char last_state[16];
} crash_record_t;

void crash_log_init();
void crash_log_write(const char *state_name, uint8_t fault_count);
void crash_log_print_last();