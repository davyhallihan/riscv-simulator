# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++11 -Wall -I/asm-files -I/formats -I/hardware-units -I/timing-utilities

# The name of the executable
TARGET := riscv-simulator

# Find all .cpp files recursively
SRCS := $(shell find $(CURDIR) -type f -name '*.cpp')

# Generate object files from source files
OBJS := $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean