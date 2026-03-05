#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

void sha1(const unsigned char *input, size_t len, unsigned char digest[20]);