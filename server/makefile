# =========================
# Configuration
# =========================

CC       := gcc
SRC_DIR  := src
DIR      := build
VERSION  := $(shell cat ../VERSION 2>/dev/null || echo "0.0.0")

ifeq ($(OS),Windows_NT)
TARGET   := $(DIR)/server.exe
SOCK_LIB := -lws2_32
MKDIR_P  := if not exist "$(DIR)" mkdir "$(DIR)"
RM_RF    := if exist "$(DIR)" rmdir /S /Q "$(DIR)"
else
TARGET   := $(DIR)/server
SOCK_LIB :=
MKDIR_P  := mkdir -p $(DIR)
RM_RF    := rm -rf $(DIR)
endif

CPPFLAGS := -I./lib -DCODENAMES_VERSION=\"$(VERSION)\"
CFLAGS   := -Wall -Wextra -O2 -MMD -MP
LDFLAGS  :=

# =========================
# Sources
# =========================

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# =========================
# Rules
# =========================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(SOCK_LIB)

# Compilation générique
$(DIR)/%.o: $(SRC_DIR)/%.c | $(DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

# Création dossier build
$(DIR):
	$(MKDIR_P)

clean:
	$(RM_RF)

-include $(DEPS)

.PHONY: all clean