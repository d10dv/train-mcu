idf_component_register(SRCS "cmd_handler.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES cjson motor_driver light_ctrl horn train_config log)
