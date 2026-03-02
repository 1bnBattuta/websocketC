/** 
 * @file main.c
 * @author MERROUN Omar
 * @brief Basic WebSocket server written by MERROUN Omar, released under MIT license
 *      Resource used:
 *           https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers
 *           https://www.rfc-editor.org/rfc/rfc6455
*/

#include <arpa/inet.h> // IP addresses ustilities
#include <netinet/in.h> // Internet addresses family and structures
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> // socket functions and strcutures
#include <unistd.h> // POSIX operating system API, includes socket closing


#define HTTP_400 "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_426 "HTTP/1.1 426 Upgrade Required\r\nUpgrade: websocket\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_505 "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"

#define BUFFER_SIZE 1024
#define PORT 443 
// connection over HTTPS to avoid complications with firewalls and/or proxies


const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char *input, int length, char *output) {
    int i, j = 0;
    unsigned char buffer[3];
    
    for (i = 0; i < length; i += 3) {
        buffer[0] = input[i];
        buffer[1] = (i + 1 < length) ? input[i + 1] : 0;
        buffer[2] = (i + 2 < length) ? input[i + 2] : 0;

        output[j++] = base64_chars[(buffer[0] >> 2) & 0x3F];
        output[j++] = base64_chars[(((buffer[0] & 0x03) << 4) | (buffer[1] >> 4) & 0x0F)];
        output[j++] = (i + 1 < length) ? base64_chars[((buffer[1] & 0x0F) << 2) | ((buffer[2] >> 6) & 0x03)] : '=';
        output[j++] = (i + 2 < length) ? base64_chars[buffer[2] & 0x3F] : '=';
    }
    output[j] = '\0'; // Null-terminate the output string
}



typedef enum {
    WS_HANDSHAKE_OK            = 0,
    WS_HANDSHAKE_BAD_METHOD    = 1,
    WS_HANDSHAKE_BAD_VERSION   = 2,
    WS_HANDSHAKE_NOT_UPGRADE   = 3,
    WS_HANDSHAKE_MISSING_KEY   = 4,
    WS_HANDSHAKE_BAD_WS_VERSION = 5,
} ws_handshake_result;


const char *handshake_error_response(ws_handshake_result result) {
    switch (result) {
        case WS_HANDSHAKE_BAD_VERSION:   return HTTP_505;
        case WS_HANDSHAKE_NOT_UPGRADE:   return HTTP_426;
        default:                         return HTTP_400;
    }
}

/**
 * @brief takes a buffer containing the HTTP request and return a boolean to indicates if it is a valid
 *        websocket handshake request to establish connection
 *        Note: this can abstracted to a http request validation function that takes a buffer and a list of
 *        headers with a list of corresponding values to compare (as you can see the same code is indeed
 *        repeated for each header) TODO
 * @param[in] buffer the buffer containing the raw http request. MUST BE NULL TERMINATED
 * @return 0 for a valid request, otherwise see the enum of results
 */
int client_handshake_verify(const char* buffer) {
    // Checking for GET requests only
    if (strncmp(buffer, "GET", 3) != 0) return 1;

    // HTTP version must be greater than (or equal to) 1.1
    char *http_version = strstr(buffer, "HTTP/");
    if (http_version == NULL) return 2;
    http_version += 5;
    if (strncmp(http_version, "1.1", 3) != 0 &&
        strncmp(http_version, "2.0", 3) != 0 &&
        strncmp(http_version, "3.0", 3) != 0) return 2;

    char *colon;

    // Checking Upgrade header
    char *upgrade = strstr(buffer, "\r\nUpgrade");
    if (upgrade == NULL) return 3;
    colon = strchr(upgrade, ':');
    if (colon == NULL) return 3;
    upgrade = colon + 1;
    while (*upgrade == ' ') upgrade++;
    if (strncasecmp(upgrade, "websocket", 9) != 0) return 3;

    // Checking Connection header
    char *connection = strstr(buffer, "\r\nConnection");
    if (connection == NULL) return 3;
    colon = strchr(connection, ':');
    if (colon == NULL) return 3;
    connection = colon + 1;
    while (*connection == ' ') connection++;
    if (strncasecmp(connection, "Upgrade", 7) != 0) return 3;

    // Checking the existance of websocket key header
    char *key = strstr(buffer, "\r\nSec-WebSocket-Key");
    if (key == NULL) return 4;
    colon = strchr(key, ':');
    if (colon == NULL) return 4;

    // Checking websocket version
    char *ws_version = strstr(buffer, "\r\nSec-WebSocket-Version");
    if (ws_version == NULL) return 5;
    colon = strchr(ws_version, ':');
    if (colon == NULL) return 5;
    ws_version = colon + 1;
    while (*ws_version == ' ') ws_version++;
    if (strncasecmp(ws_version, "13", 2) != 0) return 5;

    return 0;
}

/**
 * @brief extract the websocket key from the handshake request
 */
char *websocket_key_extract(const char *buffer) {
    // Assumes buffer has been validated by client_handshake_verify()
    // and is null-terminated

    // Does not need null checking because request is validated
    char *key = strstr(buffer, "\r\nSec-WebSocket-Key");
    char *colon = strchr(key, ':');
    key = colon + 1;
    while (*key == ' ') key++;

    char *end = strstr(key, "\r\n");
    if (end == NULL) {
        perror("Malformed key");
        return NULL;
    }

    size_t key_len = end - key;
    char *result = malloc(key_len + 1);
    if (result == NULL) {
        perror("Failed to allocate memory for the key");
        return NULL;
    }

    memcpy(result, key, key_len);
    result[key_len] = '\0';

    return result;
}


int main() {

    // Set up socket server
    int server_socket_fd;
    struct sockaddr_in server_addr;
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if ((bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
        perror("Failed to bind socket to server address");
        return -1;
    }

    if (listen(server_socket_fd, 5) < 0) {
        perror("Failed to listen on server socket");
        return -1;
    }
    printf("Server listening on port %d\n", PORT);


    // Reciving Clients
    while (1) {

        // Client socket and address
        struct sockaddr client_addr;
        socklen_t len;
        int client_fd;

        if ((client_fd = accept(server_socket_fd, &client_addr, &len)) < 0) {
            perror("Failed to establish connection with the client");
            continue;
        }

        // Buffer to get the handshake http request from the client
        char *buffer[BUFFER_SIZE + 1] = {0};
        int bytes_received;
        if ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) < 0) {
            perror("Failed to receive handshake request from client");
            goto close_socket;
        }

        buffer[bytes_received] = '\0';
        ws_handshake_result result = client_handshake_verify(buffer);
        if (result != WS_HANDSHAKE_OK) {
            perror("HTTP handshake request is not valid");
            goto send_handshake_error;
        }

        // Dynamically allocated key
        char *key = websocket_key_extract(buffer);
        if (key == NULL) {
            perror("Failed to extract key from handshake request");
            goto send_handshake_error;
        }

        // Sending handshake response back
        //char *accept_key = base64_encode(SHA1(Sec-WebSocket-Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
        char *response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";

        send_handshake_error:
            const char *response = handshake_error_response(result);
            if (send(client_fd, response, strlen(response), 0) < 0) {
                perror("Failed to send response");
            }

        close_socket:
            close(client_fd);
            continue;
    }

    return 0;
}