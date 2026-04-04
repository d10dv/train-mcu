#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"

/**
 * @brief Initialize WiFi station mode and start connection
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_init(void);

#endif // WIFI_STA_H
