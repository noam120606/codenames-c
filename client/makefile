# =========================
# Configuration
# =========================

CC       := gcc
SRC_DIR  := src
DIR      := build
TARGET   := $(DIR)/client

SDL_CFLAGS_BASE := $(shell pkg-config --cflags sdl2 2>/dev/null || ./SDL2/bin/sdl2-config --cflags 2>/dev/null || echo "-I./SDL2/include")
SDL_CFLAGS := $(SDL_CFLAGS_BASE) -I./SDL2/include

SDL_LDFLAGS_BASE := $(shell pkg-config --libs sdl2 2>/dev/null || ./SDL2/bin/sdl2-config --libs 2>/dev/null || echo "-L./SDL2/lib -lSDL2")
# Always append image/ttf/mixer libs (they may not be provided by sdl2-config)
SDL_LDFLAGS := $(SDL_LDFLAGS_BASE) -L./SDL2/lib -lSDL2_image -lSDL2_ttf -lSDL2_mixer

CPPFLAGS := -I./lib $(SDL_CFLAGS)

CPPFLAGS := -I./lib $(SDL_CFLAGS)
CFLAGS   := -Wall -Wextra -O2 -MMD -MP
LDFLAGS  := $(SDL_LDFLAGS) -lm -Wl,-rpath,'$$ORIGIN/../SDL2/lib'

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