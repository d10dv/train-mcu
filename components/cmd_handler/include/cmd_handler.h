#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include <stddef.h>

/**
 * @brief Handle a command received via MQTT
 * 
 * @param topic Topic of the message
 * @param data Message content
 * @param len Length of the message
 */
void cmd_handler_execute(const char* topic, const char* data, size_t len);

#endif // CMD_HANDLER_H
