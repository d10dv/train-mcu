#include "mqtt_client_app.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>

#include "cmd_handler.h"
#include "motor_driver.h"

static const char *TAG = "MQTT_CLIENT_APP";
static esp_mqtt_client_handle_t client;
static char s_train_id[32];
static char s_lwt_msg[128];
static char s_cmd_topic[64];
static char s_ping_topic[64];

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        motor_driver_feed_watchdog();
        
        // Subscribe to command topic
        msg_id = esp_mqtt_client_subscribe(client, s_cmd_topic, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // Subscribe to ping topic
        msg_id = esp_mqtt_client_subscribe(client, s_ping_topic, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // Announce presence
        char announce_msg[128];
        snprintf(announce_msg, sizeof(announce_msg), "{\"id\": \"%s\", \"model\": \"train-mcu\", \"fw\": \"1.0.0\"}", s_train_id);
        esp_mqtt_client_publish(client, "train/announce", announce_msg, 0, 1, 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        motor_driver_feed_watchdog();
        if (strncmp(event->topic, s_cmd_topic, event->topic_len) == 0) {
            cmd_handler_execute(event->topic, event->data, event->data_len);
        } else if (strncmp(event->topic, s_ping_topic, event->topic_len) == 0) {
            char pong_topic[64];
            snprintf(pong_topic, sizeof(pong_topic), "train/%s/pong", s_train_id);
            mqtt_app_publish(pong_topic, "{}", 0, 0);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_err_t mqtt_app_start(const char* train_id)
{
    strncpy(s_train_id, train_id, sizeof(s_train_id) - 1);
    snprintf(s_cmd_topic, sizeof(s_cmd_topic), "train/%s/cmd", s_train_id);
    snprintf(s_ping_topic, sizeof(s_ping_topic), "train/%s/ping", s_train_id);

    snprintf(s_lwt_msg, sizeof(s_lwt_msg),
             "{\"id\":\"%s\",\"model\":\"train-mcu\",\"fw\":\"1.0.0\",\"online\":false}",
             s_train_id);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_ESP_MQTT_BROKER_URL,
        .session.last_will.topic = "train/announce",
        .session.last_will.msg = s_lwt_msg,
        .session.last_will.qos = 0,
        .session.last_will.retain = 1,
        .session.keepalive = 15,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    return ESP_OK;
}

esp_err_t mqtt_app_publish(const char* topic, const char* data, int qos, int retain)
{
    if (client == NULL) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_publish(client, topic, data, -1, qos, retain);
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}
