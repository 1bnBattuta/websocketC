#include "ws_sha1.h"

/**
 * @brief Computes the SHA-1 digest of the input buffer
 * @param[in]  input  Null-terminated input string
 * @param[in]  len    Length of input in bytes
 * @param[out] digest Output buffer, must be exactly 20 bytes
 */
void sha1(const unsigned char *input, size_t len, unsigned char digest[20]) {

    // --- Stage 1: Initialize state ---
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    // --- Stage 2: Preprocessing / padding ---
    size_t padded_len = ((len + 8) / 64 + 1) * 64;
    unsigned char *msg = calloc(padded_len, 1);
    if (msg == NULL) {
        perror("Failed to allocate memory in sha1 padding (calloc)");
        return;
    }

    memcpy(msg, input, len);
    msg[len] = 0x80; // append 1 bit

    // append original length in bits as 64-bit big-endian
    uint64_t bit_len = (uint64_t)len * 8;
    for (int i = 0; i < 8; i++) {
        msg[padded_len - 1 - i] = (unsigned char)(bit_len >> (i * 8));
    }

    // --- Stage 3: Compression ---
    for (size_t chunk = 0; chunk < padded_len; chunk += 64) {
        unsigned char *block = msg + chunk;
        uint32_t w[80];

        // prepare message schedule
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint32_t)block[i*4]     << 24) |
                   ((uint32_t)block[i*4 + 1] << 16) |
                   ((uint32_t)block[i*4 + 2] <<  8) |
                   ((uint32_t)block[i*4 + 3]);
        }
        for (int i = 16; i < 80; i++) {
            w[i] = ROTL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        }

        // initialize working variables
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        // 80 rounds
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;

            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            uint32_t temp = ROTL(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = ROTL(b, 30);
            b = a;
            a = temp;
        }

        // update state
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    free(msg);

    // --- Stage 4: Produce digest (big-endian) ---
    uint32_t hash[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; i++) {
        digest[i*4]     = (hash[i] >> 24) & 0xFF;
        digest[i*4 + 1] = (hash[i] >> 16) & 0xFF;
        digest[i*4 + 2] = (hash[i] >>  8) & 0xFF;
        digest[i*4 + 3] = (hash[i])       & 0xFF;
    }
}
