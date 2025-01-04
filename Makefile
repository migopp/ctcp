# Compiler
CCOMP		= clang
CFLAGS		= -std=c99 -Wall -Wextra -g

# Directories
LIB_DIR		= lib
SRC_DIR		= .
TARGET_DIR	= target


# Source
LIB_SRC		= $(wildcard $(LIB_DIR)/*.c)
CLIENT_SRC 	= $(SRC_DIR)/client.c
SERVER_SRC 	= $(SRC_DIR)/server.c

# Targets
CLIENT	= $(TARGET_DIR)/client
SERVER	= $(TARGET_DIR)/server

all: $(CLIENT) $(SERVER)

client: $(CLIENT)
	$(TARGET_DIR)/client

server: $(SERVER)
	$(TARGET_DIR)/server

$(CLIENT): $(CLIENT_SRC) $(LIB_SRC)
	mkdir -p $(dir $@)
	$(CCOMP) $(CFLAGS) -o $@ $^

$(SERVER): $(SERVER_SRC) $(LIB_SRC)
	mkdir -p $(dir $@)
	$(CCOMP) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGET_DIR)

rebuild: clean all

.PHONY: all client server clean rebuild
