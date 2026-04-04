idf_component_register(SRCS "state_reporter.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES cjson mqtt_client_app motor_driver light_ctrl battery_monitor esp_timer esp_hw_support log)
