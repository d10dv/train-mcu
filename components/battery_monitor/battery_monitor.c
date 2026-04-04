#include "battery_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "BATTERY_MONITOR";

#define BATTERY_ADC_CHANNEL ADC_CHANNEL_0 // GPIO 36

static adc_oneshot_unit_handle_t adc_handle;
static int s_battery_pct = 100;

esp_err_t battery_monitor_init(void)
{
    ESP_LOGI(TAG, "Initializing battery monitor...");
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BATTERY_ADC_CHANNEL, &config));

    return ESP_OK;
}

int battery_monitor_get_pct(void)
{
    int adc_raw;
    if (adc_oneshot_read(adc_handle, BATTERY_ADC_CHANNEL, &adc_raw) == ESP_OK) {
        // Simplified calculation: assuming 4.2V is 100% and 3.3V is 0%
        // with a 1:1 voltage divider (4.2V -> 2.1V at ESP32 pin)
        // ADC range 0-4095 for 0-3.3V (with 11dB atten)
        // 2.1V is approx 2600 ADC raw
        // 1.65V (3.3V battery) is approx 2048 ADC raw
        s_battery_pct = (adc_raw - 2048) * 100 / (2600 - 2048);
        if (s_battery_pct > 100) s_battery_pct = 100;
        if (s_battery_pct < 0) s_battery_pct = 0;
    }
    return s_battery_pct;
}
