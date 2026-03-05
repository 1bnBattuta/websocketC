#include <stdio.h>
#include <string.h>

#include "base64/base64.h"
#include "sha1/sha1.h"

#define HTTP_400 "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_426 "HTTP/1.1 426 Upgrade Required\r\nUpgrade: websocket\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_505 "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"

typedef enum {
    WS_HANDSHAKE_OK            = 0,
    WS_HANDSHAKE_BAD_METHOD    = 1,
    WS_HANDSHAKE_BAD_VERSION   = 2,
    WS_HANDSHAKE_NOT_UPGRADE   = 3,
    WS_HANDSHAKE_MISSING_KEY   = 4,
    WS_HANDSHAKE_BAD_WS_VERSION = 5,
} ws_handshake_result;


const char *handshake_error_response(ws_handshake_result result);
void ws_accept_key(const char *client_key, char *output);
ws_handshake_result client_handshake_verify(char* buffer);
char *websocket_key_extract(char *buffer);
