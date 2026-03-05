/** 
 * @file main.c
 * @author MERROUN Omar
 * @brief Basic WebSocket server written by MERROUN Omar, released under MIT license
 *      Resource used:
 *           https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers
 *           https://www.rfc-editor.org/rfc/rfc6455
*/

#include <arpa/inet.h>  // IP addresses ustilities
#include <netinet/in.h> // Internet addresses family and structures
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> // socket functions and strcutures
#include <unistd.h>     // POSIX operating system API, includes socket closing

#include "ws_frame.h"
#include "ws_handshake.h"

#define ACCEPT_KEY_SIZE 29  // Size of accpet key buffer
#define BUFFER_SIZE 1024    // Generic buffer size
#define PORT 443 
// connection over HTTPS to avoid complications with firewalls and/or proxies


/**
 * @return the fd of server socket
 */
int server_initiate(struct sockaddr_in *server_addr) {
    // Set up socket server
    int server_socket_fd;
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        return -1;
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr->sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set server socket option: SO_REUSEADDR");
        return -1;
    } 

    if ((bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
        perror("Failed to bind socket to server address");
        return -1;
    }

    if (listen(server_socket_fd, 5) < 0) {
        perror("Failed to listen on server socket");
        return -1;
    }
    printf("Server listening on port %d\n", PORT);
    return server_socket_fd;
}

typedef struct {
    uint8_t  data[BUFFER_SIZE];
    size_t   len;           // bytes currently in buffer
} recv_buffer;

// in ws_session, instead of parsing directly from recv:
void ws_session(int client_fd) {
    recv_buffer rb = {0};

    while (1) {
        ssize_t bytes = recv(client_fd,
                             rb.data + rb.len,
                             BUFFER_SIZE - rb.len, 0);
        if (bytes <= 0) break;
        rb.len += bytes;

        ws_frame frame;
        ws_parse_result r = ws_frame_parse(rb.data, rb.len, &frame);

        if (r == WS_PARSE_INCOMPLETE) continue; // wait for more data

        if (r != WS_PARSE_OK) {
            ws_send_close(client_fd, 1002, "Protocol error");
            break;
        }

        size_t frame_total = (frame.payload - rb.data) + frame.payload_len;

        if (ws_frame_dispatch(client_fd, &frame) != 0) break;

        // shift remaining bytes to front of buffer
        rb.len -= frame_total;
        memmove(rb.data, rb.data + frame_total, rb.len);
    }
}


void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE + 1] = {0};
    int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) { perror("recv"); return; }
    buffer[bytes_received] = '\0';

    ws_handshake_result result = client_handshake_verify(buffer);
    if (result != WS_HANDSHAKE_OK) {
        const char *err = handshake_error_response(result);
        send(client_fd, err, strlen(err), 0);
        return;
    }

    char *key = websocket_key_extract(buffer);
    if (key == NULL) {
        send(client_fd, HTTP_400, strlen(HTTP_400), 0);
        return;
    }

    char accept_key[ACCEPT_KEY_SIZE];
    ws_accept_key(key, accept_key);
    free(key);

    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n",
        accept_key);

    if (send(client_fd, response, strlen(response), 0) < 0) {
        perror("send");
        return;
    }

    ws_session(client_fd);
}


int main() {

    // Set up socket server
    struct sockaddr_in *server_addr;
    int server_socket_fd = server_initiate(server_addr);
    if (server_socket_fd < 0) return -1;

    // Reciving Clients
    while (1) {

        // Client socket and address
        struct sockaddr client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd;

        if ((client_fd = accept(server_socket_fd, &client_addr, &len)) < 0) {
            perror("Failed to establish connection with the client");
            continue;
        }

        handle_client(client_fd);

        close(client_fd);
    }

    return 0;
}
