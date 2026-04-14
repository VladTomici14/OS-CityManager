# ----- setting up the compiling and flags -----
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INC_DIR)

# ----- declaring the directories -----
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# ----- declaring the target executable -----
TARGET = OS-CityManager

# ----- finding all the files from the src directory -----
SRCS = $(wildcard $(SRC_DIR)/*.c)

# ----- finding all the files from the obj directory -----
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# ----- [RULE]: build the target -----
all: $(TARGET)

# ----- [RULE]: link the object files in the final executable -----
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with ./$(TARGET)"

# ----- [RULE]: compile C files into object files -----
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ----- creating the obj directory -----
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ----- [RULE]: remove generated file -----
clean:
	rm -rf $(OBJ_DIR) $(TARGET) active_reports-*
	@echo "Cleaned up build files."

# ----- phony targets -----
.PHONY: all clean
