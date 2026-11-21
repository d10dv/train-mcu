idf_component_register(SRCS "cmd_handler.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES cjson command_queue motor_driver log)
