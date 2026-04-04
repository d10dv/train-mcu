#ifndef HORN_H
#define HORN_H

#include "esp_err.h"

/**
 * @brief Initialize horn (Buzzer PWM)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t horn_init(void);

/**
 * @brief Trigger a short horn signal
 */
void horn_short(void);

/**
 * @brief Trigger a long horn signal
 */
void horn_long(void);

/**
 * @brief Turn horn off
 */
void horn_off(void);

#endif // HORN_H
