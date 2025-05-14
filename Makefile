CC = gcc
CFLAGS = -lm -lpthread -mcmodel=medium -fPIC
SRC = src/config.c src/main.c src/hash.c src/construct.c src/sample.c src/parser.c
BIN_DIR = bin
TARGET = $(BIN_DIR)/main

# Ensure bin directory exists
$(shell mkdir -p $(BIN_DIR))

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)
