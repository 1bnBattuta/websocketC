#include "ws_handshake.h"

const char *handshake_error_response(ws_handshake_result result) {
    switch (result) {
        case WS_HANDSHAKE_BAD_VERSION:   return HTTP_505;
        case WS_HANDSHAKE_NOT_UPGRADE:   return HTTP_426;
        default:                         return HTTP_400;
    }
}

/**
 * @brief Computes the Sec-WebSocket-Accept value from a Sec-WebSocket-Key
 * @param[in]  client_key  The key extracted from the handshake request
 * @param[out] output      Buffer for the base64-encoded accept key, must be at least 29 bytes
 */
void ws_accept_key(const char *client_key, char *output) {
    static const char *GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // concatenate key + GUID
    char combined[64 + 36 + 1];
    snprintf(combined, sizeof(combined), "%s%s", client_key, GUID);

    // SHA-1 hash
    unsigned char digest[20];
    sha1((unsigned char *)combined, strlen(combined), digest);

    // base64 encode
    base64_encode(digest, 20, output);
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
ws_handshake_result client_handshake_verify(char* buffer) {
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
char *websocket_key_extract(char *buffer) {
    // Assumes buffer has been validated by client_handshake_verify()
    // and is null-terminated

    // Does not need null checking because request is validated
    char *key = strstr(buffer, "\r\nSec-WebSocket-Key");
    char *colon = strchr(key, ':');
    key = colon + 1;
    while (*key == ' ') key++;

    char *end = strstr(key, "\r\n");
    if (end == NULL) {
        fprintf(stderr, "Malformed key");
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
