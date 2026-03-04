#include <arpa/inet.h> // for ntohs
#include <endian.h> // For be64toh
#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef enum {
    WS_PARSE_OK                 = 0,
    WS_PARSE_INVALID_LEN        = 1,
    WS_PARSE_EXTENTION_ERROR    = 2,
    WS_PARSE_INVALID_OPCODE     = 3,
    WS_PARSE_UNMASKED_DATA      = 4
} ws_parse_result;

typedef struct {
    uint8_t fin_rsv_opcode; // FIN (1 bit) + Reserved (3 bit) + Opcode (4 bits)
    uint8_t mask_paylen;    // MASK (1 bit) + payload len (7 bits)
    union {
        uint16_t len16;     // if paylen == 126
        uint64_t len64;     // if paylen == 127
    } ext_payload_len;      
    uint32_t masking_key;   // Always present because every frame sent by the client
                            // must be masked (MASK = 1)
    const uint8_t *payload;                
}  ws_frame;

// The following two are internal functions
//ws_parse_result frame_parse(const uint8_t *raw_data, size_t len, ws_frame *frame);
//ws_parse_result frame_validate(const ws_frame *frame);

ws_parse_result ws_frame_parse(const uint8_t *buf, size_t len, ws_frame *frame);
