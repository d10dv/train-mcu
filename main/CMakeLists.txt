idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES nvs_flash esp_netif esp_event log
                                   train_config wifi_sta mqtt_client_app motor_driver
                                   command_queue
                                   light_ctrl horn battery_monitor state_reporter)
