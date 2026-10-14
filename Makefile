# I'm not very good at Makefile but this seems to be working.

CC = gcc

# Recursively locate files to compile in include directory.
INCLUDE_DIRS = $(shell find include -type d)

# All warnings + extra, optimization, and convert include files to -I flags.
CFLAGS = -Wall -Wextra -O2 $(addprefix -I,$(INCLUDE_DIRS))

# Libraries to link.
LIBRARIES = -llz4

SRC_DIR = src
BUILD_DIR = build
TARGET = unbig

# Recusively find all C files in the src directory.
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

# Link everything into the executable.
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBRARIES) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
