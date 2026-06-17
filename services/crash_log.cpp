#include "crash_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>

// configuration constraints for NVS
#define NVS_NAMESPACE "crash_log" // NVS partition namespace dedicated to crash logging
#define NVS_KEY "last_crash" // unique key used to store the crash record blob

/*
* initializes the crash log system and checks for any existing crash data from previous boot
*/
void crash_log_init()
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle); // open NVS namespace in read/write mode
    if(err != ESP_OK){
        log_message(LOG_ERROR, "crash_log: NVS open failed");
        return;
    }

    crash_record_t record;
    size_t size = sizeof(record);

    // attempt to retrieve the stored crash record blob from NVS
    err = nvs_get_blob(handle, NVS_KEY, &record, &size);

    if (err == ESP_OK) // valid crash record was found, print it
    {
        crash_log_print_last();
    }
    else // no previous record exists (clean previous shutdown or first boot)
    {
        log_message(LOG_INFO, "crash_log: no previous crash record");
    }

    // always close the NVS handle to free up resources
    nvs_close(handle);

}

/*
* saves the current system state and metrics into NVS right before a reboot or upon a fault
* @param state_name: string representing the last known state of the application state machine
* @param fault_count: the accumulated number of faults triggered
*/
void crash_log_write(const char *state_name, uint8_t fault_count)
{
    nvs_handle_t handle;

    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) // open NVS. abort if it fails immediately.
    {
        return;
    }   

    crash_record_t record;
    record.timestamp_sec = (uint32_t)(esp_timer_get_time() / 1000000); // convert systems uptime from microseconds to seconds
    record.reset_reason = (uint8_t)esp_reset_reason(); 
    record.fault_count = fault_count;
    strncpy(record.last_state, state_name, sizeof(record.last_state) - 1); // copy the state string
    record.last_state[sizeof(record.last_state)-1] = '\0'; // ensuring null-termination to prevent buffer overflows

    nvs_set_blob(handle, NVS_KEY, &record, sizeof(record)); // write the raw struct data as a binary blob to NVS
    nvs_commit(handle); // commit the changes to flash memory to ensure data persistence
    nvs_close(handle); // clean up and close the handle

    log_message(LOG_WARN, "crash_log: record written to NVS");
}

void crash_log_print_last()
{
    nvs_handle_t handle;

    
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) //open the NVS namespace in read-only mode for safety
    {
        return;
    }

    crash_record_t record;
    size_t size = sizeof(record);


    if (nvs_get_blob(handle, NVS_KEY, &record, &size) == ESP_OK) // read the binary blob dat into the local record struct
    {
        printf("=== LAST CRASH RECORD ===\n");
        printf("  Uptime at crash: %lu s\n", record.timestamp_sec);
        printf("  Reset reason: %d\n", record.reset_reason);
        printf("  Fault count: %d\n", record.fault_count);
        printf("  Last state: %s\n", record.last_state);
        printf("=========================\n");
    }

    nvs_close(handle); //close the handle to prevent leaks

}