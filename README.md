# websocketC

A lightweight, zero-dependency WebSocket server written in C from scratch, implementing [RFC 6455](https://datatracker.ietf.org/doc/html/rfc6455).

## Features

- RFC 6455 compliant WebSocket handshake ( stand by )
- Manual SHA-1 implementation (no OpenSSL dependency)
- Manual Base64 implementation
- Zero-copy frame parsing
- TCP reassembly buffer (handles fragmented reads)
- One POSIX thread per client connection
- Correct ping/pong and close frame handling

## Build

```bash
make
```

## Run

```bash
sudo ./build/ws_server   # sudo required for port 443
```

Or change `PORT` in `include/ws_server.h` to an unprivileged port (e.g. 9090).

## Test

```bash
# run unit tests
make test

# connect with websocat
websocat ws://localhost:443
```

## Project Structure

```
include/        public headers — one per module
src/            implementations
tests/          unit tests per module
docs/           architecture/tests notes
```

## Architecture

See [docs/architecture.md](docs/architecture.md) for design decisions and internals.

## License
Written by MERROUN Omar under MIT
