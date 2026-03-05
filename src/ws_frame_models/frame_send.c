#include "ws_frame_models/frame_send.h"

static int ws_send_frame(int fd, uint8_t opcode, const uint8_t *payload, size_t len) {
    uint8_t header[10];
    size_t header_len;

    header[0] = 0x80 | opcode; // FIN + opcode

    if (len <= 125) {
        header[1] = (uint8_t)len;
        header_len = 2;
    } else if (len <= 65535) {
        header[1] = 126;
        header[2] = (len >> 8) & 0xFF;
        header[3] = len & 0xFF;
        header_len = 4;
    } else {
        header[1] = 127;
        for (int i = 0; i < 8; i++)
            header[2 + i] = (len >> (56 - i * 8)) & 0xFF;
        header_len = 10;
    }

    // server frames are never masked per RFC 6455
    if (send(fd, header, header_len, 0) < 0) return -1;
    if (payload && len > 0)
        if (send(fd, payload, len, 0) < 0) return -1;

    return 0;
}

int ws_send_text(int fd, const uint8_t *payload, size_t len) {
    return ws_send_frame(fd, 0x1, payload, len);
}

int ws_send_binary(int fd, const uint8_t *payload, size_t len) {
    return ws_send_frame(fd, 0x2, payload, len);
}

int ws_send_pong(int fd, const uint8_t *payload, size_t len) {
    return ws_send_frame(fd, 0xA, payload, len);
}

int ws_send_close(int fd, uint16_t code, const char *reason) {
    uint8_t payload[125];
    size_t len = 0;

    if (code) {
        payload[0] = (code >> 8) & 0xFF;
        payload[1] = code & 0xFF;
        len = 2;
        if (reason) {
            size_t reason_len = strlen(reason);
            if (reason_len > 123) reason_len = 123;
            memcpy(payload + 2, reason, reason_len);
            len += reason_len;
        }
    }

    return ws_send_frame(fd, 0x8, payload, len);
}
