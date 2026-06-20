#include "mqtt_client_service.h"
#include "mqtt_client.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

static esp_mqtt_client_handle_t client = NULL; // opaque tracking pointer holding the hidden internal state of the MQTT instance
static bool mqtt_connected = false; // synchronous application state flag for safe data publishing checkpoints

/*
* Asynchronous background callback executed by the system event loop
* Invoked whenever the MQTT protocol engine experiences a state transition.
*/
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data; //type-cast the generic void* payloadinto the specific MQTT structure template
    // ^cannot simply do event_data->event_id because it is of type void*
    switch (event->event_id) // evaluate the specific event packet ID delivered by the network engine
    {
        case MQTT_EVENT_CONNECTED;
            mqtt_connected = true; // set thread state flag to allow data transmission
            log_message(LOG_INFO, "MQTT connected");
            break;
        case MQTT_EVENT_DISCONNECTED;
            mqtt_connected = false; // clamp transmissions to prevent memory/socket errors
            log_message(LOG_WARN, "MQTT disconnected");
            break;
        default:
            break;
    }
}

/*
* allocates data profiles, maps subscription hooks, and executes the MQTT connection thread
*/
void mqtt_service_init(const char *broker_uri)
{
    esp_mqtt_client_config_t config = {}; // define an empty stack allocation profile for operational properties
    config.broker.address.uri = broker_uri; // feed the generic target broker URI address string

    client = esp_mqtt_client_init(&config); // initialize driver engine memory blocks and store the access key pointer
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handle, NULL); // register our local event_handler filter directly to the client's internal loop
    esp_mqtt_client_start(client); // asynchronously wake up the underlying network background socket task; essentially spawns a dedicated background thread (called mqtt_task)
    log_message(LOG_INFO, "MQTT client started");
}

/*
* takes raw sensor data(internal BME280 metrics) and converts it into JSON text format
* and pushes it across the network to your MQTT server.
*/
void mqtt_publish_telemetry(const sensor_reading_t *reading, const char *state_name, int fault_count)
{

    // fail fast if the socket layer is offline
    if (!mqtt_connected || client == NULL) 
    {
        return;
    }

    char payload[128]; // buffer space to protect against stack overwrites

    // serialize structure values into a compact machine-readable JSON string array
    snprintf(payload, sizeof(payload),
        "{\"temp\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f,\"state\":\"%s\",\"faults\":%d}",
        reading->temperature, reading->humidity, reading->pressure,
        state_name, fault_count);
    
    // ship packet over the broker pipeline (client, Topic, data, length=auto, QoS = 1, Retain = 0)
    esp_mqtt_client_publish(client, "headless_node/telemetry", payload, 0, 1 ,0);
}

/*
* immediately push structural alert strings to the dedicated fault logger topic
*/
void mqtt_publish_fault(const char *reason)
{
    if (!mqtt_connected || client == NULL) // protect network pipes against disconnected socket calls
    {
        return;
    }

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"fault\":\"%s\"}", reason);
    esp_mqtt_client_publish(client, "headless_node_tjcdy/fault", payload, 0, 1, 0); // publish warning tracking alerts with QoS is 1
}

// return the current network state
bool mqtt_service_is_connected()
{
    return mqtt_connected;
}