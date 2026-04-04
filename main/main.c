#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "train_config.h"
#include "wifi_sta.h"
#include "mqtt_client_app.h"
#include "motor_driver.h"
#include "light_ctrl.h"
#include "horn.h"
#include "battery_monitor.h"
#include "state_reporter.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Train MCU firmware...");

    // 1. NVS Flash init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. TCP/IP stack init
    ESP_ERROR_CHECK(esp_netif_init());

    // 3. Event loop init
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 4. train_config — загрузка конфигурации из NVS
    ESP_ERROR_CHECK(train_config_init());
    const train_config_t* cfg = train_config_get();
    ESP_LOGI(TAG, "Train ID: %s", cfg->id);

    // 5. motor_driver — инициализация PWM, H-мост
    ESP_ERROR_CHECK(motor_driver_init());

    // 6. light_ctrl — инициализация GPIO
    ESP_ERROR_CHECK(light_ctrl_init());

    // 7. horn — инициализация PWM
    ESP_ERROR_CHECK(horn_init());

    // 8. battery_monitor — запуск периодического опроса ADC
    ESP_ERROR_CHECK(battery_monitor_init());

    // 9. wifi_sta — подключение к AP
    ESP_ERROR_CHECK(wifi_sta_init());

    // 10. mqtt_client — подключение к брокеру, announce, подписки
    ESP_ERROR_CHECK(mqtt_app_start(cfg->id));

    // 11. state_reporter — запуск периодической публикации состояния
    ESP_ERROR_CHECK(state_reporter_start(cfg->id));

    ESP_LOGI(TAG, "All systems go.");
}
