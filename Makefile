# Configuration
CC   = gcc
SHDC = util/sokol-shdc

CFLAGS = -pthread -Wall -Wextra -Iinclude -Ilib -I$(BUILD_DIR)
LIBS   = -pthread -lX11 -lXi -lXcursor -ldl -lpthread -lm -lGL

# Directories
SRC_DIR    = src
BUILD_DIR  = build
BIN_DIR    = bin
SHADER_DIR = data/shaders

# Files
SRCS        = $(wildcard $(SRC_DIR)/*.c)
OBJS        = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
SHADERS     = $(wildcard $(SHADER_DIR)/*.glsl)
SHADER_HDRS = $(patsubst $(SHADER_DIR)/%.glsl, $(BUILD_DIR)/%.glsl.h, $(SHADERS))
APP = $(BIN_DIR)/app

# Targets
.PHONY: all shaders run clean

all: $(SHADER_HDRS) $(APP)

shaders: $(SHADER_HDRS)

run: all
	./$(APP)

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f  $(APP)

# Rules
$(BUILD_DIR) $(BIN_DIR):
	@mkdir -p $@

$(APP): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SHADER_HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.glsl.h: $(SHADER_DIR)/%.glsl | $(BUILD_DIR)
	$(SHDC) -i $< -o $@ -l glsl430
