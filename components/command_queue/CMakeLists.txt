idf_component_register(SRCS "command_queue.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES motor_driver light_ctrl horn train_config log freertos)
