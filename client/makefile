# =========================
# Configuration
# =========================

CC       := gcc
SRC_DIR  := src
DIR      := build
VERSION  := $(shell cat ../VERSION 2>/dev/null || echo "0.0.0")

ifeq ($(OS),Windows_NT)
TARGET   := $(DIR)/client.exe
EXTRA_NET_LIBS := -lws2_32
RPATH_FLAGS :=
MKDIR_P  := if not exist "$(DIR)" mkdir "$(DIR)"
RM_RF    := if exist "$(DIR)" rmdir /S /Q "$(DIR)"
else
TARGET   := $(DIR)/client
EXTRA_NET_LIBS :=
RPATH_FLAGS := -Wl,-rpath,'$$ORIGIN/../SDL2/lib'
MKDIR_P  := mkdir -p $(DIR)
RM_RF    := rm -rf $(DIR)
endif

SDL_CFLAGS_BASE := $(shell pkg-config --cflags sdl2 2>/dev/null || ./SDL2/bin/sdl2-config --cflags 2>/dev/null || echo "-I./SDL2/include")
SDL_CFLAGS := $(SDL_CFLAGS_BASE) -I./SDL2/include

SDL_LDFLAGS_BASE := $(shell pkg-config --libs sdl2 2>/dev/null || ./SDL2/bin/sdl2-config --libs 2>/dev/null || echo "-L./SDL2/lib -lSDL2")
# Always append image/ttf/mixer libs (they may not be provided by sdl2-config)
SDL_LDFLAGS := $(SDL_LDFLAGS_BASE) -L./SDL2/lib -lSDL2_image -lSDL2_ttf -lSDL2_mixer

CPPFLAGS := -D_GNU_SOURCE -I./lib $(SDL_CFLAGS) -DCODENAMES_VERSION=\"$(VERSION)\"
CFLAGS   := -Wall -Wextra -O2 -MMD -MP -finput-charset=UTF-8
LDFLAGS  := $(SDL_LDFLAGS) -lm $(RPATH_FLAGS) $(EXTRA_NET_LIBS)

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
	$(MKDIR_P)

clean:
	$(RM_RF)

-include $(DEPS)

.PHONY: all clean