# =========================
# Configuration
# =========================

CC       := gcc
SRC_DIR  := src
DIR      := build
TARGET   := $(DIR)/client

SDL_CFLAGS  := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer 2>/dev/null)
SDL_LDFLAGS := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_ttf SDL2_mixer 2>/dev/null)

CPPFLAGS := -I./lib $(SDL_CFLAGS)
CFLAGS   := -Wall -Wextra -O2 -MMD -MP
LDFLAGS  := $(SDL_LDFLAGS) -lm

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

# Création dossier build si nécessaire
$(DIR):
	mkdir -p $(DIR)

clean:
	rm -rf $(DIR)

-include $(DEPS)

.PHONY: all clean