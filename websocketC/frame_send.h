#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> 

int ws_send_text(int fd, const uint8_t *payload, size_t len);
int ws_send_binary(int fd, const uint8_t *payload, size_t len);
int ws_send_pong(int fd, const uint8_t *payload, size_t len);
int ws_send_close(int fd, uint16_t code, const char *reason);
