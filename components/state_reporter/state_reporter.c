#include "state_reporter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "mqtt_client_app.h"
#include "motor_driver.h"
#include "light_ctrl.h"
#include "battery_monitor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "STATE_REPORTER";
static char s_train_id[32];
static char s_state_topic[64];

static void state_reporter_task(void *pvParameters)
{
    while (1) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "speed", motor_driver_get_current_speed());
        cJSON_AddStringToObject(root, "direction", (motor_driver_get_direction() == DIRECTION_FORWARD) ? "forward" : "reverse");
        
        cJSON *lights = cJSON_AddObjectToObject(root, "lights");
        cJSON_AddBoolToObject(lights, "head", light_ctrl_get_state(LIGHT_HEAD));
        cJSON_AddBoolToObject(lights, "tail", light_ctrl_get_state(LIGHT_TAIL));
        cJSON_AddBoolToObject(lights, "cabin", light_ctrl_get_state(LIGHT_CABIN));
        
        cJSON_AddNumberToObject(root, "battery", battery_monitor_get_pct());
        cJSON_AddNullToObject(root, "error");
        cJSON_AddNumberToObject(root, "uptime", (int)(esp_timer_get_time() / 1000000));

        char *json_str = cJSON_PrintUnformatted(root);
        mqtt_app_publish(s_state_topic, json_str, 0, 1);
        
        free(json_str);
        cJSON_Delete(root);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

esp_err_t state_reporter_start(const char* train_id)
{
    strncpy(s_train_id, train_id, sizeof(s_train_id) - 1);
    snprintf(s_state_topic, sizeof(s_state_topic), "train/%s/state", s_train_id);
    
    xTaskCreate(state_reporter_task, "state_reporter", 4096, NULL, 5, NULL);
    return ESP_OK;
}
