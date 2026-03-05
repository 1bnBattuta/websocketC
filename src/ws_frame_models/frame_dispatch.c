#include "frame_dispatch.h"

void ws_payload_unmask(ws_frame *frame) {
    uint8_t *payload = (uint8_t *) frame->payload;
    for (size_t i = 0; i < frame->payload_len; i++) {
        payload[i] ^= frame->masking_key[i % 4];
    }
}

/**
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
    // echo back for now — replace with your actual logic
    ws_send_text(client_fd, frame->payload, frame->payload_len);
    return 0;
}

int ws_handle_binary(int client_fd, ws_frame *frame) {
    ws_payload_unmask(frame);
    ws_send_binary(client_fd, frame->payload, frame->payload_len);
    return 0;
}

int ws_handle_ping(int client_fd, ws_frame *frame) {
    // must respond with pong carrying the same payload, per RFC 6455
    ws_payload_unmask(frame);
    ws_send_pong(client_fd, frame->payload, frame->payload_len);
    return 0;
}

int ws_handle_pong(int client_fd, ws_frame *frame) {
    // nothing to do — pong is just an acknowledgement
    (void)client_fd; (void)frame;
    return 0;
}

int ws_handle_close(int client_fd, ws_frame *frame) {
    // echo the close frame back then terminate, per RFC 6455
    ws_payload_unmask(frame);
    ws_send_close(client_fd, 1000, NULL);
    return -1;
}

int ws_handle_continuation(int client_fd, ws_frame *frame) {
    // TODO: fragmentation support
    (void)client_fd; (void)frame;
    return -1;
}
