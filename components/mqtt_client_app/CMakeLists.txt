idf_component_register(SRCS "mqtt_client_app.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES mqtt cmd_handler motor_driver log)
