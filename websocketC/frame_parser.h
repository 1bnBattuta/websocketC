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
    WS_PARSE_UNMASKED_DATA      = 4,
    WS_PARSE_INCOMPLETE         = 5
} ws_parse_result;

/**
 * Represents a parsed WebSocket frame.
 * Note: this struct does not map directly to the wire format due to alignment.
 * Note: payload points into the original recv buffer and is only valid for
 *       its lifetime. Payload data is still masked — use ws_frame_unmask()
 *       before reading it.
 */
typedef struct {
    uint8_t        fin_rsv_opcode;
    uint8_t        mask_paylen;
    uint64_t       payload_len;
    uint8_t        masking_key[4];
    const uint8_t *payload;
} ws_frame;

// The following two are internal functions
//ws_parse_result frame_parse(const uint8_t *raw_data, size_t len, ws_frame *frame);
//ws_parse_result frame_validate(const ws_frame *frame);

ws_parse_result ws_frame_parse(const uint8_t *buf, size_t len, ws_frame *frame);
