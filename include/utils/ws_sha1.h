#ifndef WS_SHA1_H
#define WS_SHA1_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

void ws_sha1(const unsigned char *input, size_t len, unsigned char digest[20]);

#endif
