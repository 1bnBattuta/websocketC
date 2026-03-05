CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Iinclude
SRC     = $(wildcard src/*.c) $(wildcard src/utils/*.c) $(wildcard src/ws_frame_models/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)
TARGET  = build/ws_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

build/%.o: src/%.c | build
	@mkdir -p $(dir $@)   # creates build/frame/ if needed
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

tests: $(wildcard tests/*.c)
	$(CC) $(CFLAGS) -o build/tests $^ src/ws_handshake.c src/ws_frame.c -lpthread
	./build/tests

clean:
	rm -rf build