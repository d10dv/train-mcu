#ifndef MQTT_CLIENT_APP_H
#define MQTT_CLIENT_APP_H

#include "esp_err.h"

/**
 * @brief Initialize MQTT client and start connection
 * 
 * @param train_id The unique ID of the train
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_app_start(const char* train_id);

/**
 * @brief Publish a message to a topic
 * 
 * @param topic The topic to publish to
 * @param data The message content
 * @param qos Quality of Service level
 * @param retain Retain flag
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_app_publish(const char* topic, const char* data, int qos, int retain);

#endif // MQTT_CLIENT_APP_H
