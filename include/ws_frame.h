/**
 * main frame API, providing dispatch, parse and send utilities
 */
#ifndef WS_FRAME_H
#define WS_FRAME_H

#include "ws_frame_models/frame_dispatch.h"
#include "ws_frame_models/frame_parser.h"
#include "ws_frame_models/frame_send.h"

// Dispatch
void ws_payload_unmask(ws_frame *frame);
int ws_frame_dispatch(int client_fd, ws_frame *frame);
int ws_handle_continuation(int client_fd, ws_frame *frame);
int ws_handle_text(int client_fd, ws_frame *frame);
int ws_handle_binary(int client_fd, ws_frame *frame);
int ws_handle_close(int client_fd, ws_frame *frame);
int ws_handle_ping(int client_fd, ws_frame *frame);
int ws_handle_pong(int client_fd, ws_frame *frame);

// Parse
ws_parse_result ws_frame_parse(const uint8_t *buf, size_t len, ws_frame *frame);

// Send
int ws_send_text(int fd, const uint8_t *payload, size_t len);
int ws_send_binary(int fd, const uint8_t *payload, size_t len);
int ws_send_pong(int fd, const uint8_t *payload, size_t len);
int ws_send_close(int fd, uint16_t code, const char *reason);

#endif
