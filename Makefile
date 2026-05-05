# ----- setting up the compiling and flags -----
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INC_DIR)

# ----- declaring the directories -----
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# ----- declaring the target executable -----
TARGET_CITY = OS-CityManager
TARGET_MONITOR = monitor_reports

# ----- finding all the files from the src directory -----
SRCS_ALL = $(wildcard $(SRC_DIR)/*.c)
SRCS_MONITOR = $(SRC_DIR)/monitor_reports.c
SRCS_CITY = $(filter-out $(SRCS_MONITOR), $(SRCS_ALL))

# ----- finding all the files from the obj directory -----
OBJS_CITY = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_CITY))
OBJS_MONITOR = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_MONITOR))

# ----- [RULE]: build the target -----
all: $(TARGET_CITY) $(TARGET_MONITOR)

# ----- [RULE]: link the object files in the final executable -----
$(TARGET_CITY): $(OBJS_CITY)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with ./$(TARGET_CITY)"

$(TARGET_MONITOR): $(OBJS_MONITOR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with ./$(TARGET_MONITOR)"

# ----- [RULE]: compile C files into object files -----
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ----- creating the obj directory -----
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ----- [RULE]: remove generated file -----
clean:
	rm -rf $(OBJ_DIR) $(TARGET_CITY) $(TARGET_MONITOR) active_reports-* .monitor_pid
	@echo "Cleaned up build files."

# ----- phony targets -----
.PHONY: all clean
