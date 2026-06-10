# Makefile for Analog Devices ADXL380 C++ Driver Example

CXX = clang++
CC = clang
CXXFLAGS = -Wall -Wextra -g -std=c++11 -I../../lib/Cpp -I../../lib/C
CFLAGS_C_COMPAT = -Wall -Wextra -g -I../../lib/C
LDFLAGS =

# Source files
C_LIB_SRCS = ../../lib/C/adxl380_driver.c
CPP_LIB_SRCS = ../../lib/Cpp/adxl380_driver.cpp
EXAMPLE_SRCS = main.cpp

# Build directory
BUILD_DIR = build

# Object files
C_LIB_OBJS = $(BUILD_DIR)/$(notdir $(C_LIB_SRCS:.c=.o))
CPP_LIB_OBJS = $(BUILD_DIR)/$(notdir $(CPP_LIB_SRCS:.cpp=.o))
EXAMPLE_OBJS = $(BUILD_DIR)/$(notdir $(EXAMPLE_SRCS:.cpp=.o))
OBJS = $(EXAMPLE_OBJS) $(CPP_LIB_OBJS) $(C_LIB_OBJS)

# Target executable
TARGET_NAME = adxl380_cpp_example
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: ../../lib/C/%.c ../../lib/C/adxl380_driver.h | $(BUILD_DIR)
	$(CC) $(CFLAGS_C_COMPAT) -c $< -o $@

$(BUILD_DIR)/adxl380_driver.o: ../../lib/Cpp/adxl380_driver.cpp ../../lib/Cpp/adxl380_driver.hpp ../../lib/C/adxl380_driver.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c ../../lib/Cpp/adxl380_driver.cpp -o $@

$(BUILD_DIR)/main.o: main.cpp ../../lib/Cpp/adxl380_driver.hpp ../../lib/C/adxl380_driver.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
