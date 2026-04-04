#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include "esp_err.h"

/**
 * @brief Initialize battery monitor (ADC)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t battery_monitor_init(void);

/**
 * @brief Get battery percentage (0-100)
 */
int battery_monitor_get_pct(void);

#endif // BATTERY_MONITOR_H
