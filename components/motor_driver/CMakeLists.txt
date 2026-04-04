idf_component_register(SRCS "motor_driver.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc train_config log)
