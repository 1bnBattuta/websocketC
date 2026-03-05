#include "ws_frame_models/frame_dispatch.h"

void ws_payload_unmask(ws_frame *frame) {
    uint8_t *payload = (uint8_t *) frame->payload;
    for (size_t i = 0; i < frame->payload_len; i++) {
        payload[i] ^= frame->masking_key[i % 4];
    }
}

/**
 * @brief Dispatches a parsed, unmasked frame to the appropriate handler.
 * @param[in] client_fd  Connected client socket file descriptor
 * @param[in] frame      Parsed and unmasked WebSocket frame
 * @return 0 to continue session, -1 to terminate
 */
int ws_frame_dispatch(int client_fd, ws_frame *frame) {
    uint8_t opcode = frame->fin_rsv_opcode & 0x0F;
    switch (opcode) {
    case 0x0: return ws_handle_continuation(client_fd, frame);
    case 0x1: return ws_handle_text(client_fd, frame);
    case 0x2: return ws_handle_binary(client_fd, frame);
    case 0x8: return ws_handle_close(client_fd, frame);
    case 0x9: return ws_handle_ping(client_fd, frame);
    case 0xA: return ws_handle_pong(client_fd, frame);
    
    default:
        ws_send_close(client_fd, 1002, "Unknown opcode");
        return -1;
    }
}

int ws_handle_text(int client_fd, ws_frame *frame) {
    ws_payload_unmask(frame);
    printf("[text] %.*s\n", (int)frame->payload_len, frame->payload);
    return ws_send_text(client_fd, frame->payload, frame->payload_len);
}

int ws_handle_binary(int client_fd, ws_frame *frame) {
    ws_payload_unmask(frame);
    printf("[binary] %zu bytes\n", frame->payload_len);
    return ws_send_binary(client_fd, frame->payload, frame->payload_len);
}

int ws_handle_ping(int client_fd, ws_frame *frame) {
    ws_payload_unmask(frame);
    printf("[ping]\n");
    return ws_send_pong(client_fd, frame->payload, frame->payload_len);
}

int ws_handle_pong(int client_fd, ws_frame *frame) {
    // pong is just an acknowledgement, nothing to do
    (void)client_fd;
    (void)frame;
    printf("[pong]\n");
    return 0;
}

int ws_handle_close(int client_fd, ws_frame *frame) {
    ws_payload_unmask(frame);
    printf("[close]\n");
    ws_send_close(client_fd, 1000, NULL);
    return -1; // signal session end
}

int ws_handle_continuation(int client_fd, ws_frame *frame) {
    // TODO: fragmentation support
    (void)client_fd;
    (void)frame;
    fprintf(stderr, "[continuation] fragmented frames not yet supported\n");
    return -1;
}
