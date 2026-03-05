# Architecture

## Overview

This server implements the WebSocket protocol (RFC 6455) from scratch in C with zero external dependencies. The goal was to understand the protocol internals rather than delegate to a library.

## Module breakdown

### utils

SHA-1 and Base64 are the only cryptographic primitives required by the WebSocket handshake. Both are implemented manually.

SHA-1 follows RFC 3174 exactly: message padding, 512-bit block decomposition, an 80-word message schedule, and four rounds of 20 compression operations each. The implementation is validated against the RFC test vectors before integration.

Base64 operates on 3-byte groups, producing 4 output characters per group with `=` padding for incomplete final groups.


### ws_handshake

Parses and validates the HTTP upgrade request. Uses `strstr` and `strchr` rather than a full HTTP parser since the only request type this server accepts is a WebSocket upgrade — a well-defined, predictable format. The buffer must be null-terminated by the caller (enforced after `recv`), which allows `strstr` to operate safely without a separate length bound.

Validation returns a typed enum (`ws_handshake_result`) rather than a boolean so the caller can send a semantically correct HTTP error response (400, 426, or 505) rather than a generic failure.


### ws_frame

Parses WebSocket frames from raw bytes. Uses a **zero-copy** approach: the parsed `ws_frame` struct holds a pointer directly into the `recv` buffer rather than copying the payload. This avoids a `malloc` per frame on the hot path.

The frame parser distinguishes between `WS_PARSE_INCOMPLETE` (not enough data yet — normal under TCP) and `WS_PARSE_INVALID_LEN` (malformed frame — close the connection). This distinction is important for the session loop's TCP reassembly logic.

Payload unmasking is a separate explicit step (`ws_unmask_payload`) rather than happening automatically during parsing. This keeps the parser pure and makes the ownership contract clear: the caller unmasks in-place when ready to consume the data.

Ws_frame is an internal API grouping three files: frame_parser, frame_dispatch, frame_send. 

Frame dispatch is table-driven via a switch on the opcode, making it straightforward to add new opcode handlers.


### ws_session

Manages the per-client session loop. Maintains a reassembly buffer to handle TCP fragmentation — a WebSocket frame may arrive split across multiple `recv` calls, or multiple frames may arrive in one call. After each successfully parsed frame, consumed bytes are shifted out of the buffer with `memmove`.


### ws_server

Owns the socket lifecycle (bind, listen, accept) and spawns a `pthread` per accepted connection. The client fd is heap-allocated before being passed to the thread so there is no race between the accept loop reading the fd and the thread starting. Threads are detached so they clean themselves up on exit.


## Design decisions

**Zero dependency** — The only things linked are libc and libpthread. This makes the build trivially portable and removes any version or license concerns.

**Zero-copy frame parsing** — Avoiding a `malloc` per frame matters at scale. The tradeoff is that the caller must not reuse or free the recv buffer while a frame is live.

**Thread-per-client over epoll** — For a demo server, `pthread` per client is simpler to reason about and debug. An epoll-based event loop would scale better but adds significant complexity without benefit at this scale.

**Manual SHA-1 over OpenSSL** — The handshake hashes a single short string once per connection. OpenSSL's EVP interface would add a dependency for a one-line operation. The manual implementation is ~70 lines and directly validates against the RFC.