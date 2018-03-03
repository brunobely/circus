SRC = ./src
OBJ = ./obj
BIN = ./bin
DIRS = $(SRC) $(OBJ) $(BIN)
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
HEADERS = $(wildcard $(SRC)/*.h)

TARGET = $(BIN)/circus
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic

.PHONY: default all clean run debug dirs

default: dirs $(TARGET)
all: default

dirs: $(DIRS)

$(DIRS):
	mkdir -p $@

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

run: $(TARGET)
	$(TARGET)