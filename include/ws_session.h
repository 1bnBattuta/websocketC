#ifndef WS_SESSION_H
#define WS_SESSION_H

#include "ws_frame.h"
#include "ws_server.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

/**
 * @brief Runs the WebSocket session loop for a connected client.
 *        Blocks until the session ends (client disconnect, close frame, or error).
 *        Handles frame dispatch, unmasking, ping/pong, and close handshake.
 * @param[in] client_fd  Connected client socket file descriptor
 */
void ws_session(int client_fd);

#endif
