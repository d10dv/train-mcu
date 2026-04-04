#include "horn.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "HORN";

#define HORN_GPIO 19
#define HORN_TIMER              LEDC_TIMER_1
#define HORN_MODE               LEDC_LOW_SPEED_MODE
#define HORN_CHANNEL            LEDC_CHANNEL_1
#define HORN_DUTY_RES           LEDC_TIMER_10_BIT
#define HORN_FREQUENCY          (2000) // 2 kHz tone

esp_err_t horn_init(void)
{
    ESP_LOGI(TAG, "Initializing horn...");
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = HORN_MODE,
        .timer_num        = HORN_TIMER,
        .duty_resolution  = HORN_DUTY_RES,
        .freq_hz          = HORN_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = HORN_MODE,
        .channel        = HORN_CHANNEL,
        .timer_sel      = HORN_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = HORN_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    return ESP_OK;
}

static void horn_task(void *pvParameters)
{
    int duration_ms = (int)pvParameters;
    ledc_set_duty(HORN_MODE, HORN_CHANNEL, 512); // 50% duty
    ledc_update_duty(HORN_MODE, HORN_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(HORN_MODE, HORN_CHANNEL, 0);
    ledc_update_duty(HORN_MODE, HORN_CHANNEL);
    vTaskDelete(NULL);
}

void horn_short(void)
{
    xTaskCreate(horn_task, "horn_task", 2048, (void*)200, 5, NULL);
}

void horn_long(void)
{
    xTaskCreate(horn_task, "horn_task", 2048, (void*)800, 5, NULL);
}

void horn_off(void)
{
    ledc_set_duty(HORN_MODE, HORN_CHANNEL, 0);
    ledc_update_duty(HORN_MODE, HORN_CHANNEL);
}
