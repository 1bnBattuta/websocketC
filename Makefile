CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Iinclude
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)
TARGET  = build/ws_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

tests: $(wildcard tests/*.c)
	$(CC) $(CFLAGS) -o build/tests $^ src/ws_handshake.c src/ws_frame.c src/ws_crypto.c -lpthread
	./build/tests

clean:
	rm -rf build