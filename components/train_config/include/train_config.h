#ifndef TRAIN_CONFIG_H
#define TRAIN_CONFIG_H

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    char id[32];
    char model[32];
    int32_t accel_rate;
    int32_t decel_rate;
    int32_t max_speed;
} train_config_t;

/**
 * @brief Initialize NVS and load configuration
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t train_config_init(void);

/**
 * @brief Get the current configuration
 * 
 * @return const train_config_t* Pointer to config structure
 */
const train_config_t* train_config_get(void);

/**
 * @brief Update a configuration parameter and save to NVS
 * 
 * @param param Parameter name
 * @param value New value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t train_config_set_int(const char* param, int32_t value);

#endif // TRAIN_CONFIG_H
