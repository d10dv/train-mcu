#include "cmd_handler.h"
#include "command_queue.h"
#include "cJSON.h"
#include "esp_log.h"
#include "motor_driver.h"
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

    cmd_t cmd = { .action = CMD_ACTION_NONE, .value = 0, .text = {0} };

    if (strcmp(action_str, "speed") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsNumber(value)) {
            cmd.action = CMD_ACTION_SPEED;
            cmd.value = value->valueint;
        }
    } else if (strcmp(action_str, "stop") == 0) {
        cmd.action = CMD_ACTION_STOP;
    } else if (strcmp(action_str, "estop") == 0) {
        // Highest priority: stop the motors immediately, then make sure
        // nothing queued before or after can restart the train.
        motor_driver_estop();
        command_queue_flush();
        command_queue_cancel_plan();
    } else if (strcmp(action_str, "direction") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(value)) {
            cmd.action = CMD_ACTION_DIRECTION;
            strncpy(cmd.text, value->valuestring, sizeof(cmd.text) - 1);
        }
    } else if (strcmp(action_str, "light") == 0) {
        cJSON *target = cJSON_GetObjectItem(root, "target");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(target) && cJSON_IsBool(value)) {
            cmd.action = CMD_ACTION_LIGHT;
            cmd.value = cJSON_IsTrue(value) ? 1 : 0;
            strncpy(cmd.text, target->valuestring, sizeof(cmd.text) - 1);
        }
    } else if (strcmp(action_str, "horn") == 0) {
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(value)) {
            cmd.action = CMD_ACTION_HORN;
            strncpy(cmd.text, value->valuestring, sizeof(cmd.text) - 1);
        }
    } else if (strcmp(action_str, "config") == 0) {
        cJSON *param = cJSON_GetObjectItem(root, "param");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        if (cJSON_IsString(param) && cJSON_IsNumber(value)) {
            cmd.action = CMD_ACTION_CONFIG;
            cmd.value = value->valueint;
            strncpy(cmd.text, param->valuestring, sizeof(cmd.text) - 1);
        }
    }

    cJSON_Delete(root);

    if (cmd.action == CMD_ACTION_NONE) {
        // Unknown action or malformed payload — nothing to enqueue.
        return;
    }
    esp_err_t err = command_queue_post(&cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to post command %d: %s", cmd.action, esp_err_to_name(err));
    }
}
