# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./json/include

# Directories and files
SRC = Raytracer.cpp
TARGET = Raytracer
JSON_INPUT = ../json_list/scene.json
OUTPUT_DIR = ../Image_output
NORMAL_OUTPUT = $(OUTPUT_DIR)/scene_normal1.ppm
LINEAR_OUTPUT = $(OUTPUT_DIR)/scene_linear1.ppm

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Build target
all: $(TARGET)

# Link and build the executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run the program with user input prompt
run: $(TARGET) $(OUTPUT_DIR)
	./$(TARGET) $(JSON_INPUT) $(NORMAL_OUTPUT) $(LINEAR_OUTPUT) 

# Clean up generated files
clean:
	rm -f $(TARGET) $(OUTPUT_DIR)/*.ppm

# Phony targets
.PHONY: all run clean