#include "ws_server.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int server_fd = server_init(PORT);
    if (server_fd < 0) {
        fprintf(stderr, "Failed to initialize server\n");
        return EXIT_FAILURE;
    }

    server_run(server_fd);

    return EXIT_SUCCESS;
}