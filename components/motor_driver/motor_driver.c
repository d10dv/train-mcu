#include "motor_driver.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "train_config.h"
#include <stdlib.h>

static const char *TAG = "MOTOR_DRIVER";

#define MOTOR_IN1_GPIO 25
#define MOTOR_IN2_GPIO 26
#define MOTOR_ENA_GPIO 27

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // 0-1023
#define LEDC_FREQUENCY          (5000) // 5 kHz

static int s_target_speed = 0;
static int s_current_speed = 0;
static train_direction_t s_direction = DIRECTION_FORWARD;
static uint32_t s_last_feed_time = 0;
static bool s_safety_enabled = true;

static void motor_update_hw(void)
{
    if (s_current_speed == 0) {
        gpio_set_level(MOTOR_IN1_GPIO, 0);
        gpio_set_level(MOTOR_IN2_GPIO, 0);
    } else {
        if (s_direction == DIRECTION_FORWARD) {
            gpio_set_level(MOTOR_IN1_GPIO, 1);
            gpio_set_level(MOTOR_IN2_GPIO, 0);
        } else {
            gpio_set_level(MOTOR_IN1_GPIO, 0);
            gpio_set_level(MOTOR_IN2_GPIO, 1);
        }
    }

    const train_config_t* cfg = train_config_get();
    uint32_t duty = (s_current_speed * ((1 << LEDC_DUTY_RES) - 1) * cfg->max_speed) / 10000;
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void motor_task(void *pvParameters)
{
    s_last_feed_time = xTaskGetTickCount();
    while (1) {
        if (s_safety_enabled && s_current_speed > 0) {
            if ((xTaskGetTickCount() - s_last_feed_time) > pdMS_TO_TICKS(5000)) {
                ESP_LOGW(TAG, "Safety stop: connection lost!");
                s_target_speed = 0;
            }
        }

        const train_config_t* cfg = train_config_get();
        int rate = (s_target_speed > s_current_speed) ? cfg->accel_rate : cfg->decel_rate;
        
        if (s_current_speed != s_target_speed) {
            if (abs(s_target_speed - s_current_speed) <= rate) {
                s_current_speed = s_target_speed;
            } else {
                s_current_speed += (s_target_speed > s_current_speed) ? rate : -rate;
            }
            motor_update_hw();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t motor_driver_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MOTOR_IN1_GPIO) | (1ULL << MOTOR_IN2_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_ENA_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    motor_update_hw();

    xTaskCreate(motor_task, "motor_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}

void motor_driver_set_speed(int speed_pct)
{
    if (speed_pct < 0) speed_pct = 0;
    if (speed_pct > 100) speed_pct = 100;
    s_target_speed = speed_pct;
    ESP_LOGI(TAG, "Target speed set to %d", s_target_speed);
}

esp_err_t motor_driver_set_direction(train_direction_t dir)
{
    if (s_current_speed > 0) {
        ESP_LOGW(TAG, "Cannot change direction while moving!");
        return ESP_ERR_INVALID_STATE;
    }
    s_direction = dir;
    motor_update_hw();
    ESP_LOGI(TAG, "Direction set to %s", (dir == DIRECTION_FORWARD) ? "FORWARD" : "REVERSE");
    return ESP_OK;
}

void motor_driver_estop(void)
{
    s_target_speed = 0;
    s_current_speed = 0;
    motor_update_hw();
    ESP_LOGW(TAG, "ESTOP triggered!");
}

void motor_driver_feed_watchdog(void)
{
    s_last_feed_time = xTaskGetTickCount();
}

void motor_driver_set_safety_enabled(bool enabled)
{
    s_safety_enabled = enabled;
}

int motor_driver_get_current_speed(void) { return s_current_speed; }
train_direction_t motor_driver_get_direction(void) { return s_direction; }
