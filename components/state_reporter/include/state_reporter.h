#ifndef STATE_REPORTER_H
#define STATE_REPORTER_H

#include "esp_err.h"

/**
 * @brief Start the state reporter task
 * 
 * @param train_id Unique ID of the train
 * @return esp_err_t ESP_OK on success
 */
esp_err_t state_reporter_start(const char* train_id);

#endif // STATE_REPORTER_H
