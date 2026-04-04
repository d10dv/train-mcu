#include "cmd_handler.h"
#include "cJSON.h"
#include "esp_log.h"
#include "motor_driver.h"
#include "light_ctrl.h"
#include "horn.h"
#include "train_config.h"
#include <string.h>

static const char *TAG = "CMD_HANDLER";

void cmd_handler_execute(const char* topic, const char* data, size_t len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON: %.*s", (int)len, data);
        return;
    }

    cJSON *action = cJSON_GetObjectItem(root, "action");
    if (!cJSON_IsString(action)) {
        ESP_LOGE(TAG, "Missing 'action' field in command");
        cJSON_Delete(root);
        return;
    }

    const char* action_str = action->valuestring;
    ESP_LOGI(TAG, "Handling action: %s", action_str);

    if (strcmp(action_str, "speed") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsNumber(value)) {
            motor_driver_set_speed(value->valueint);
        }
    } else if (strcmp(action_str, "stop") == 0) {
        motor_driver_set_speed(0);
    } else if (strcmp(action_str, "estop") == 0) {
        motor_driver_estop();
    } else if (strcmp(action_str, "direction") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(value)) {
            train_direction_t dir = (strcmp(value->valuestring, "forward") == 0) ? DIRECTION_FORWARD : DIRECTION_REVERSE;
            motor_driver_set_direction(dir);
        }
    } else if (strcmp(action_str, "light") == 0) {
        cJSON *target = cJSON_GetObjectItem(root, "target");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(target) && cJSON_IsBool(value)) {
            light_target_t lt;
            if (strcmp(target->valuestring, "head") == 0) lt = LIGHT_HEAD;
            else if (strcmp(target->valuestring, "tail") == 0) lt = LIGHT_TAIL;
            else if (strcmp(target->valuestring, "cabin") == 0) lt = LIGHT_CABIN;
            else if (strcmp(target->valuestring, "all") == 0) lt = LIGHT_ALL;
            else {
                cJSON_Delete(root);
                return;
            }
            light_ctrl_set(lt, cJSON_IsTrue(value));
        }
    } else if (strcmp(action_str, "horn") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(value)) {
            if (strcmp(value->valuestring, "short") == 0) horn_short();
            else if (strcmp(value->valuestring, "long") == 0) horn_long();
            else if (strcmp(value->valuestring, "off") == 0) horn_off();
        }
    } else if (strcmp(action_str, "config") == 0) {
        cJSON *param = cJSON_GetObjectItem(root, "param");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(param) && cJSON_IsNumber(value)) {
            train_config_set_int(param->valuestring, value->valueint);
        }
    }

    cJSON_Delete(root);
}
