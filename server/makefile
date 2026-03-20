# =========================
# Configuration
# =========================

CC       := gcc
SRC_DIR  := src
DIR      := build
TARGET   := $(DIR)/server
VERSION  := $(shell cat ../VERSION 2>/dev/null || echo "0.0.0")

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
	$(CC) $^ -o $@ $(LDFLAGS)

# Compilation générique
$(DIR)/%.o: $(SRC_DIR)/%.c | $(DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

# Création dossier build
$(DIR):
	mkdir -p $(DIR)

clean:
	rm -rf $(DIR)

-include $(DEPS)

.PHONY: all clean