# Makefile for Analog Devices ADXL380 C Driver Example

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I../../lib/C
LDFLAGS =

# Source files
LIB_SRCS = ../../lib/C/adxl380_driver.c
EXAMPLE_SRCS = main.c

# Build directory
BUILD_DIR = build

# Object files
LIB_OBJS = $(BUILD_DIR)/$(notdir $(LIB_SRCS:.c=.o))
EXAMPLE_OBJS = $(BUILD_DIR)/$(notdir $(EXAMPLE_SRCS:.c=.o))
OBJS = $(EXAMPLE_OBJS) $(LIB_OBJS)

# Target executable
TARGET_NAME = adxl380_c_example
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: ../../lib/C/%.c ../../lib/C/adxl380_driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c ../../lib/C/adxl380_driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
