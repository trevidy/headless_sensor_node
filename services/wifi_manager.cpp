#include "wifi_manager.h"
#include "logger.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

/*
* an event group uses an entire collection of bits. Depends on how the ESP32 is configured, but almost always 24 bits.
*/
#define WIFI_CONNECTED_BIT BIT0 // Binary: 000000000000000000000001 (Decimal 1)
#define WIFI_FAIL_BIT      BIT1 // Binary: 000000000000000000000010 (Decimal 1)
// initial state: Binary: 000000000000000000000000 (both bits off) 

#define MAX_RETRIES        5

/*
* EventGroupHandle_t is a datatype that represents a reference pointer to an event group structure
*/
static EventGroupHandle_t wifi_event_group; // create an event group structure for wifi, create flags for connection or unable to connect to network.
static int retry_count = 0;
static bool connected = false;

/*
* Central background callback handler for raw ESP32 wi-fi and ip-stack events.
* Runs in the context of the default system event loop thread
* Event handler's job: if you hear X happens, then execute Y action.
*/
static void event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // handle wifi disconnect events (failure or mid-session drop)
    if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        connected = false;
        if (retry_count < MAX_RETRIES)
        {
            esp_wifi_connect(); // attempt non-blocking reassociation
            retry_count++;
            printf("WiFi disconnected, retrying (%d/%d)", retry_count, MAX_RETRIES);

        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT); // after max retries, unblock initialization thread with a failure status
        }
    }

    // handle successful IP acquisition event from the DHCP client
    else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data; // we confirmed IP_EVENT_STA_GOT_IP occurs, so the payload is always ip_event_got_ip_t structure.
        printf("WiFi connected. IP: " IPSTR "\n", IP2STR(&event->ip_info.ip)); // output assigned IP using the IPSTR formatting macros
        retry_count = 0;
        connected = true;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT); // unblock initialization thread with a successful status
    }
}

/*
* Synchronously initializes the network interfaces and starts the station connection sequence.
* Blocks for up to 10 seconds waiting for authentication and IP assignment.
*/
void wifi_manager_init(const char *ssid, const char *password)
{
    wifi_event_group = xEventGroupCreate(); // create the synchronization primitive for tracking the network handshake

    // 1. Initialize the underlying LwIP network stack layer
    esp_netif_init();

    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();

    // wait up to 10s for connection
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT)
    {   
        log_message(LOG_INFO, "WiFi connected");
    }
    else
    {
        log_message(LOG_WARN, "WiFi connection failed - running offline");
    }
}

bool wifi_manager_is_connected()
{
    return connected;
}