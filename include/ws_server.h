#ifndef WS_SERVER_H
#define WS_SERVER_H

#include "ws_handshake.h"
#include "ws_session.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT        443
#define BUFFER_SIZE 1024
#define ACCEPT_KEY_SIZE 30

/**
 * @brief Initializes and binds the server socket.
 * @param[in] port  Port to listen on
 * @return Server socket file descriptor, or -1 on error
 */
int server_init(int port);

/**
 * @brief Starts the server accept loop. Blocks indefinitely.
 *        Spawns a new thread per client connection.
 * @param[in] server_fd  A bound and listening server socket fd
 */
void server_run(int server_fd);

/**
 * @brief Handles a single client connection: handshake then session.
 *        Intended to be run in its own thread.
 * @param[in] arg  Heap-allocated int* containing the client fd. Freed internally.
 */
void *handle_client(void *arg);

#endif
