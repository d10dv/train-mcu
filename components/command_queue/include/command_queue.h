#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    CMD_ACTION_NONE = 0,
    CMD_ACTION_SPEED,     // value: speed 0-100
    CMD_ACTION_STOP,      // no params (speed 0)
    CMD_ACTION_ESTOP,     // handled immediately in cmd_handler, never queued
    CMD_ACTION_DIRECTION, // text: "forward" / "reverse"
    CMD_ACTION_LIGHT,     // text: head|tail|cabin|all, value: 0/1
    CMD_ACTION_HORN,      // text: short|long|off
    CMD_ACTION_CONFIG     // text: param name, value: number
} cmd_action_t;

typedef struct {
    cmd_action_t action;
    int value;
    char text[24];
    uint32_t epoch; // internal: commands from an older epoch are dropped
} cmd_t;

/**
 * @brief Create queue and start the dispatcher task.
 *
 * Must be called before any command_queue_post().
 */
esp_err_t command_queue_init(void);

/**
 * @brief Post a parsed command for ordered execution by the dispatcher.
 *
 * SPEED commands are coalesced: if an unconsumed SPEED is already in the
 * queue, its value is replaced in place (position and ordering preserved).
 * If the queue is full, the oldest command is dropped (with a warning).
 *
 * @param cmd Command to enqueue (copied by value)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t command_queue_post(const cmd_t *cmd);

/**
 * @brief Drop all pending commands and invalidate already-queued ones.
 *
 * Called on estop so no stale command can restart the train afterwards.
 */
void command_queue_flush(void);

/**
 * @brief Drop a queued "change direction when stopped" plan, if any.
 *
 * Called on estop so the train stays put instead of resuming a deferred
 * direction change.
 */
void command_queue_cancel_plan(void);

#endif // COMMAND_QUEUE_H
