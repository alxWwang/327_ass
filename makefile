# Makefile for project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I./header -Wno-unused-parameter

# Directories
OBJ_DIR = obj

# Source files (all .c files in src/)
SRC = src/main.c \
      src/djikstras.c \
      src/minheap.c \
      src/monsters.c \
      src/utils.c \
      src/map.c \
      src/load_file.c \
      src/save_file.c

# Object files (store them in obj/)
OBJ = $(SRC:src/%.c=$(OBJ_DIR)/%.o)

# Executable output
TARGET = ./game

# Libraries (e.g., math)
LIBS = -lm

# Default rule: compile and link
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Compile .c to .o, placing object files in obj/
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up the object and binary files
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
# Phony targets
.PHONY: clean
