#include "command_queue.h"
#include "motor_driver.h"
#include "light_ctrl.h"
#include "horn.h"
#include "train_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "COMMAND_QUEUE";

#define QUEUE_DEPTH           8
#define DISPATCHER_TIMEOUT_MS 100

static cmd_t s_buf[QUEUE_DEPTH];
static int s_head = 0;  // next write index
static int s_tail = 0;  // next read index
static int s_count = 0;
static SemaphoreHandle_t s_mutex;
static TaskHandle_t s_dispatcher_task = NULL;

// Bumped by command_queue_flush(); commands carrying an older epoch are
// discarded by the dispatcher instead of being executed.
static uint32_t s_epoch = 0;

// Deferred direction change: set when a direction command arrives while the
// train is still moving. Applied by the dispatcher as soon as speed == 0.
static bool s_plan_pending = false;
static train_direction_t s_plan_dir = DIRECTION_FORWARD;

static bool queue_pop(cmd_t *out)
{
    bool got = false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_count > 0) {
        *out = s_buf[s_tail];
        s_tail = (s_tail + 1) % QUEUE_DEPTH;
        s_count--;
        got = true;
    }
    xSemaphoreGive(s_mutex);
    return got;
}

static void execute_command(const cmd_t *cmd)
{
    switch (cmd->action) {
    case CMD_ACTION_SPEED:
        motor_driver_set_speed(cmd->value);
        break;

    case CMD_ACTION_STOP:
        motor_driver_set_speed(0);
        break;

    case CMD_ACTION_DIRECTION: {
        train_direction_t dir = (strcmp(cmd->text, "forward") == 0)
                                    ? DIRECTION_FORWARD
                                    : DIRECTION_REVERSE;
        if (motor_driver_set_direction(dir) == ESP_ERR_INVALID_STATE) {
            // Still moving: defer and apply automatically once stopped.
            s_plan_pending = true;
            s_plan_dir = dir;
            ESP_LOGI(TAG, "Direction %s deferred until stop", cmd->text);
        } else {
            s_plan_pending = false;
        }
        break;
    }

    case CMD_ACTION_LIGHT: {
        light_target_t lt;
        if (strcmp(cmd->text, "head") == 0) lt = LIGHT_HEAD;
        else if (strcmp(cmd->text, "tail") == 0) lt = LIGHT_TAIL;
        else if (strcmp(cmd->text, "cabin") == 0) lt = LIGHT_CABIN;
        else if (strcmp(cmd->text, "all") == 0) lt = LIGHT_ALL;
        else break;
        light_ctrl_set(lt, cmd->value != 0);
        break;
    }

    case CMD_ACTION_HORN:
        if (strcmp(cmd->text, "short") == 0) horn_short();
        else if (strcmp(cmd->text, "long") == 0) horn_long();
        else if (strcmp(cmd->text, "off") == 0) horn_off();
        break;

    case CMD_ACTION_CONFIG:
        train_config_set_int(cmd->text, cmd->value);
        break;

    default:
        ESP_LOGW(TAG, "Unknown action %d, ignored", cmd->action);
        break;
    }
}

static void dispatcher_task(void *pvParameters)
{
    while (1) {
        // Wake immediately on new commands, otherwise poll periodically to
        // service a deferred direction plan once the train has stopped.
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(DISPATCHER_TIMEOUT_MS));

        if (s_plan_pending && motor_driver_get_current_speed() == 0) {
            s_plan_pending = false;
            motor_driver_set_direction(s_plan_dir);
            ESP_LOGI(TAG, "Deferred direction applied: %s",
                     (s_plan_dir == DIRECTION_FORWARD) ? "forward" : "reverse");
        }

        cmd_t cmd;
        while (queue_pop(&cmd)) {
            if (cmd.epoch != s_epoch) {
                ESP_LOGW(TAG, "Dropping stale %d command (queue flushed)", cmd.action);
                continue;
            }
            execute_command(&cmd);
        }
    }
}

esp_err_t command_queue_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) return ESP_ERR_NO_MEM;

    BaseType_t ok = xTaskCreate(dispatcher_task, "cmd_dispatcher", 2560, NULL, 5,
                                &s_dispatcher_task);
    if (ok != pdPASS) return ESP_ERR_NO_MEM;

    ESP_LOGI(TAG, "Command queue ready (depth %d)", QUEUE_DEPTH);
    return ESP_OK;
}

esp_err_t command_queue_post(const cmd_t *cmd)
{
    if (s_mutex == NULL) return ESP_ERR_INVALID_STATE;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    // Coalesce: replace an unconsumed SPEED in place so a fast knob sweep
    // cannot stack up lagging speed commands.
    if (cmd->action == CMD_ACTION_SPEED) {
        for (int i = 0; i < s_count; i++) {
            int idx = (s_tail + i) % QUEUE_DEPTH;
            if (s_buf[idx].action == CMD_ACTION_SPEED) {
                s_buf[idx].value = cmd->value;
                s_buf[idx].epoch = s_epoch;
                xSemaphoreGive(s_mutex);
                return ESP_OK;
            }
        }
    }

    if (s_count == QUEUE_DEPTH) {
        ESP_LOGW(TAG, "Queue full, dropping oldest command (%d)", s_buf[s_tail].action);
        s_tail = (s_tail + 1) % QUEUE_DEPTH;
        s_count--;
    }

    s_buf[s_head] = *cmd;
    s_buf[s_head].epoch = s_epoch;
    s_head = (s_head + 1) % QUEUE_DEPTH;
    s_count++;
    xSemaphoreGive(s_mutex);

    // Wake the dispatcher (notification count self-corrects: a flush leaves
    // spurious notifications, but every wake drains the whole ring).
    if (s_dispatcher_task) xTaskNotifyGive(s_dispatcher_task);
    return ESP_OK;
}

void command_queue_flush(void)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_head = s_tail = s_count = 0;
    s_epoch++;
    xSemaphoreGive(s_mutex);
    ESP_LOGW(TAG, "Queue flushed (epoch %u)", s_epoch);
}

void command_queue_cancel_plan(void)
{
    s_plan_pending = false;
}
