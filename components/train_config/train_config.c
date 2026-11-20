#include "train_config.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
#include <string.h>

static const char *TAG = "TRAIN_CONFIG";
static train_config_t s_config;

static void generate_unique_id(char *id_buf, size_t buf_size)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    // Format: L-XXXXXX (8 characters total) where XXXXXX are the last 3 bytes of MAC
    snprintf(id_buf, buf_size, "L-%02x%02x%02x", mac[3], mac[4], mac[5]);
}

esp_err_t train_config_init(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    // Set defaults
    generate_unique_id(s_config.id, sizeof(s_config.id));
    
    strcpy(s_config.model, "steam");
    s_config.accel_rate = 50;
    s_config.decel_rate = 50;
    s_config.max_speed = 100;
    s_config.min_duty = 40;

    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle! Using defaults.", esp_err_to_name(err));
        return ESP_OK;
    }

    size_t required_size;
    
    required_size = sizeof(s_config.id);
    if (nvs_get_str(handle, "id", s_config.id, &required_size) != ESP_OK) {
        nvs_set_str(handle, "id", s_config.id);
    }

    required_size = sizeof(s_config.model);
    if (nvs_get_str(handle, "model", s_config.model, &required_size) != ESP_OK) {
        nvs_set_str(handle, "model", s_config.model);
    }

    nvs_get_i32(handle, "accel_rate", &s_config.accel_rate);
    nvs_get_i32(handle, "decel_rate", &s_config.decel_rate);
    nvs_get_i32(handle, "max_speed", &s_config.max_speed);
    nvs_get_i32(handle, "min_duty", &s_config.min_duty);

    nvs_close(handle);
    return ESP_OK;
}

const train_config_t* train_config_get(void)
{
    return &s_config;
}

esp_err_t train_config_set_int(const char* param, int32_t value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    if (strcmp(param, "accel_rate") == 0) {
        s_config.accel_rate = value;
        err = nvs_set_i32(handle, "accel_rate", value);
    } else if (strcmp(param, "decel_rate") == 0) {
        s_config.decel_rate = value;
        err = nvs_set_i32(handle, "decel_rate", value);
    } else if (strcmp(param, "max_speed") == 0) {
        s_config.max_speed = value;
        err = nvs_set_i32(handle, "max_speed", value);
    } else if (strcmp(param, "min_duty") == 0) {
        if (value < 0) value = 0;
        if (value > 95) value = 95;
        s_config.min_duty = value;
        err = nvs_set_i32(handle, "min_duty", value);
    } else {
        err = ESP_ERR_NOT_FOUND;
    }

    if (err == ESP_OK) {
        nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
