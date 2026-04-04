#ifndef LIGHT_CTRL_H
#define LIGHT_CTRL_H

#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    LIGHT_HEAD,
    LIGHT_TAIL,
    LIGHT_CABIN,
    LIGHT_ALL
} light_target_t;

/**
 * @brief Initialize light control (GPIO)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t light_ctrl_init(void);

/**
 * @brief Set light state
 * 
 * @param target Light target (HEAD/TAIL/CABIN/ALL)
 * @param state ON/OFF
 */
void light_ctrl_set(light_target_t target, bool state);

/**
 * @brief Get current state of a light
 */
bool light_ctrl_get_state(light_target_t target);

#endif // LIGHT_CTRL_H
