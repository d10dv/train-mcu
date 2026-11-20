#include "light_ctrl.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "LIGHT_CTRL";

#define LIGHT_HEAD_GPIO 32
#define LIGHT_TAIL_GPIO 33
#define LIGHT_CABIN_GPIO 13

static bool s_head_state = false;
static bool s_tail_state = false;
static bool s_cabin_state = false;

esp_err_t light_ctrl_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LIGHT_HEAD_GPIO) | (1ULL << LIGHT_TAIL_GPIO) | (1ULL << LIGHT_CABIN_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);
    
    gpio_set_level(LIGHT_HEAD_GPIO, 0);
    gpio_set_level(LIGHT_TAIL_GPIO, 0);
    gpio_set_level(LIGHT_CABIN_GPIO, 0);

    return ESP_OK;
}

void light_ctrl_set(light_target_t target, bool state)
{
    int val = state ? 1 : 0;
    switch (target) {
        case LIGHT_HEAD:
            s_head_state = state;
            gpio_set_level(LIGHT_HEAD_GPIO, val);
            break;
        case LIGHT_TAIL:
            s_tail_state = state;
            gpio_set_level(LIGHT_TAIL_GPIO, val);
            break;
        case LIGHT_CABIN:
            s_cabin_state = state;
            gpio_set_level(LIGHT_CABIN_GPIO, val);
            break;
        case LIGHT_ALL:
            s_head_state = s_tail_state = s_cabin_state = state;
            gpio_set_level(LIGHT_HEAD_GPIO, val);
            gpio_set_level(LIGHT_TAIL_GPIO, val);
            gpio_set_level(LIGHT_CABIN_GPIO, val);
            break;
    }
    ESP_LOGI(TAG, "Light %d set to %d", target, state);
}

bool light_ctrl_get_state(light_target_t target)
{
    switch (target) {
        case LIGHT_HEAD: return s_head_state;
        case LIGHT_TAIL: return s_tail_state;
        case LIGHT_CABIN: return s_cabin_state;
        default: return false;
    }
}
