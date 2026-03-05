#include "frame_parser.h"
#include "frame_send.h"

void ws_payload_unmask(ws_frame *frame);
int ws_frame_dispatch(int client_fd, ws_frame *frame);
int ws_handle_continuation(int client_fd, ws_frame *frame);
int ws_handle_text(int client_fd, ws_frame *frame);
int ws_handle_binary(int client_fd, ws_frame *frame);
int ws_handle_close(int client_fd, ws_frame *frame);
int ws_handle_ping(int client_fd, ws_frame *frame);
int ws_handle_pong(int client_fd, ws_frame *frame);