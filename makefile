# Makefile for project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include -Wno-unused-parameter

# Directories
OBJ_DIR = obj

# Source files
SRC = src/main.c src/linked_list.c src/utils.c src/map.c src/load_file.c src/save_file.c

# Object files
OBJ = $(SRC:src/%.c=$(OBJ_DIR)/%.o)

# Executable output
TARGET = ./game

# Libraries (e.g., math)
LIBS = -lm

# Compile and link the program
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Compile source files to object files
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up the object and binary files
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET) dungeon.dat

# Phony targets
.PHONY: clean
