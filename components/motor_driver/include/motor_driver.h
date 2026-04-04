#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdint.h>
#include "esp_err.h"

typedef enum {
    DIRECTION_FORWARD,
    DIRECTION_REVERSE
} train_direction_t;

/**
 * @brief Initialize motor driver (GPIO and PWM)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t motor_driver_init(void);

/**
 * @brief Set target speed
 * 
 * @param speed_pct Speed in percent (0-100)
 */
void motor_driver_set_speed(int speed_pct);

/**
 * @brief Set direction
 * 
 * @param dir Direction (FORWARD/REVERSE)
 * @return esp_err_t ESP_OK if direction changed, ESP_ERR_INVALID_STATE if speed > 0
 */
esp_err_t motor_driver_set_direction(train_direction_t dir);

/**
 * @brief Immediate stop (estop)
 */
void motor_driver_estop(void);

/**
 * @brief Get current speed and direction
 */
/**
 * @brief Reset the connectivity watchdog timer
 */
void motor_driver_feed_watchdog(void);

/**
 * @brief Enable/disable connectivity safety stop
 */
void motor_driver_set_safety_enabled(bool enabled);

int motor_driver_get_current_speed(void);
train_direction_t motor_driver_get_direction(void);

#endif // MOTOR_DRIVER_H
