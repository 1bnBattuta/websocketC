#include "ws_session.h"


void ws_session(int client_fd) {
    // Reassembly buffer to handle TCP fragmentation:
    // a single WebSocket frame may arrive split across multiple recv() calls,
    // or multiple frames may arrive in one recv() call.
    uint8_t buf[BUFFER_SIZE];
    size_t  buffered = 0;

    while (1) {
        ssize_t bytes = recv(client_fd, buf + buffered, BUFFER_SIZE - buffered, 0);

        if (bytes < 0) { perror("recv"); break; }
        if (bytes == 0) { printf("client disconnected\n"); break; }

        buffered += (size_t)bytes;

        ws_frame frame;
        ws_parse_result r = ws_frame_parse(buf, buffered, &frame);

        if (r == WS_PARSE_INCOMPLETE) continue; // wait for more data

        if (r != WS_PARSE_OK) {
            fprintf(stderr, "frame parse error: %d\n", r);
            ws_send_close(client_fd, 1002, "Protocol error");
            break;
        }

        // compute total frame size to know how many bytes to consume
        size_t frame_total = (size_t)(frame.payload - buf) + frame.payload_len;

        if (ws_frame_dispatch(client_fd, &frame) != 0) break;

        // shift unconsumed bytes to the front of the buffer
        buffered -= frame_total;
        if (buffered > 0)
            memmove(buf, buf + frame_total, buffered);
    }
}
