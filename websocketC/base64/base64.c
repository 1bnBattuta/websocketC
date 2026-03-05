#include "base64.h"

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char *input, int length, char *output) {
    int i, j = 0;
    unsigned char buffer[3];
    
    for (i = 0; i < length; i += 3) {
        buffer[0] = input[i];
        buffer[1] = (i + 1 < length) ? input[i + 1] : 0;
        buffer[2] = (i + 2 < length) ? input[i + 2] : 0;

        output[j++] = base64_chars[(buffer[0] >> 2) & 0x3F];
        output[j++] = base64_chars[(((buffer[0] & 0x03) << 4) | (buffer[1] >> 4) & 0x0F)];
        output[j++] = (i + 1 < length) ? base64_chars[((buffer[1] & 0x0F) << 2) | ((buffer[2] >> 6) & 0x03)] : '=';
        output[j++] = (i + 2 < length) ? base64_chars[buffer[2] & 0x3F] : '=';
    }
    output[j] = '\0'; // Null-terminate the output string
}
