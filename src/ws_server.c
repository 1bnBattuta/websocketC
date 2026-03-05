#include "ws_server.h"

// ---------------------------------------------------------------------------
// Server init
// ---------------------------------------------------------------------------

int server_init(int port) {
    int server_fd;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port        = htons((uint16_t)port),
    };

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind socket");
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Failed to listen on socket");
        return -1;
    }

    printf("Server listening on port %d\n", port);
    return server_fd;
}

// ---------------------------------------------------------------------------
// Client handler (runs in its own thread)
// ---------------------------------------------------------------------------

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    // Receive handshake request
    char buffer[BUFFER_SIZE + 1] = {0};
    ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes < 0) {
        perror("recv");
        goto cleanup;
    }
    buffer[bytes] = '\0';

    // Validate handshake
    ws_handshake_result result = client_handshake_verify(buffer);
    if (result != WS_HANDSHAKE_OK) {
        fprintf(stderr, "Invalid handshake request (code %d)\n", result);
        const char *err = handshake_error_response(result);
        send(client_fd, err, strlen(err), 0);
        goto cleanup;
    }

    // Extract and process key
    char *key = websocket_key_extract(buffer);
    if (key == NULL) {
        send(client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
        goto cleanup;
    }

    char accept_key[ACCEPT_KEY_SIZE];
    ws_accept_key(key, accept_key);
    free(key);

    // Send 101 Switching Protocols
    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n",
        accept_key);

    if (send(client_fd, response, strlen(response), 0) < 0) {
        perror("send");
        goto cleanup;
    }

    printf("Client connected — WebSocket session started\n");

    // Run WebSocket session loop
    ws_session(client_fd);

    printf("Client disconnected\n");

cleanup:
    close(client_fd);
    return NULL;
}

// ---------------------------------------------------------------------------
// Accept loop
// ---------------------------------------------------------------------------

void server_run(int server_fd) {
    while (1) {
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);

        int *client_fd = malloc(sizeof(int));
        if (client_fd == NULL) {
            perror("malloc");
            continue;
        }

        *client_fd = accept(server_fd, &client_addr, &client_len);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create");
            free(client_fd);
            continue;
        }

        // detach so thread cleans up itself on exit
        pthread_detach(thread);
    }
}
