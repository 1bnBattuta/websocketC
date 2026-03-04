#include "frame_parser.h"

/**
 * @param[in] buffer the buffer containing raw data from request
 * @param[in] len the size of request buffer
 * @param[out] frame the frame structure containing parsed request data
 * Note: this is a ZERO-COPY approach, the payload stored in the frame is pointing to
 *  an address in the original buffer, so the original buffer MUST LIVE.
 *  Also payload must be unmasked before reading (in place or into a separate buffer)
 */
static ws_parse_result frame_parse(const uint8_t *buffer, size_t len, ws_frame *frame) {
    size_t read = 0;
    if (len < 6) { return WS_PARSE_INVALID_LEN; }

    frame->fin_rsv_opcode = buffer[read++];
    frame->mask_paylen    = buffer[read++];

    uint8_t payload_len_indicator = frame->mask_paylen & 0x7F;
    uint64_t actual_len = payload_len_indicator;

    switch (payload_len_indicator) {
        case 127:
            if (len < read + 8) { return WS_PARSE_INVALID_LEN; }
            memcpy(&actual_len, buffer + read, 8);
            actual_len = be64toh(actual_len);
            read += 8;
            break;
        case 126:
            if (len < read + 2) { return WS_PARSE_INVALID_LEN; }
            uint16_t len16;
            memcpy(&len16, buffer + read, 2);
            actual_len = ntohs(len16);
            read += 2;
            break;
    }

    if (len < read + 4) { return WS_PARSE_INVALID_LEN; }
    memcpy(frame->masking_key, buffer + read, 4);
    read += 4;

    if (len < read + actual_len) { return WS_PARSE_INCOMPLETE; }

    frame->payload_len = actual_len;
    frame->payload     = buffer + read;

    return WS_PARSE_OK;
}

static ws_parse_result frame_validate(const ws_frame *frame) {
    // TODO: Add support for extensions
    // For now, no extension will be supported: RSV fields must be 0
    uint8_t rsv = frame->fin_rsv_opcode & 0x70;
    if (rsv != 0) { return WS_PARSE_EXTENTION_ERROR; }

    uint8_t opcode = frame->fin_rsv_opcode & 0x0F;
    if ((opcode > 0x2 && opcode < 0x8) || opcode > 0xA) {
        return WS_PARSE_INVALID_OPCODE;
    }

    uint8_t mask = frame->mask_paylen & 0x80;
    if (mask != 0x80) { return WS_PARSE_UNMASKED_DATA; }

    return WS_PARSE_OK;
}

ws_parse_result ws_frame_parse(const uint8_t *buf, size_t len, ws_frame *frame) {
    ws_parse_result r = frame_parse(buf, len, frame);
    if (r != WS_PARSE_OK) return r;
    return frame_validate(frame);
}
