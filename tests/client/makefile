# Makefile pour tests/client/game.c
# Compile le test en liant les sources du client (../../client/src)

CC := gcc
CLIENT_DIR := ../../client

SDL_CFLAGS_BASE := $(shell pkg-config --cflags sdl2 2>/dev/null || $(CLIENT_DIR)/SDL2/bin/sdl2-config --cflags 2>/dev/null || echo "-I$(CLIENT_DIR)/SDL2/include")
SDL_CFLAGS := $(SDL_CFLAGS_BASE) -I$(CLIENT_DIR)/SDL2/include

SDL_LDFLAGS_BASE := $(shell pkg-config --libs sdl2 2>/dev/null || $(CLIENT_DIR)/SDL2/bin/sdl2-config --libs 2>/dev/null || echo "-L$(CLIENT_DIR)/SDL2/lib -lSDL2")
SDL_LDFLAGS := $(SDL_LDFLAGS_BASE) -L$(CLIENT_DIR)/SDL2/lib -lSDL2_image -lSDL2_ttf -lSDL2_mixer

CPPFLAGS := -I$(CLIENT_DIR)/lib $(SDL_CFLAGS)
CFLAGS := -Wall -Wextra -O2 -MMD -MP

SRC_DIR := $(CLIENT_DIR)/src
# Récupère toutes les sources mais exclut index.c (qui contient un main)
ALL_SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(filter-out $(SRC_DIR)/index.c,$(ALL_SRCS))

OBJ_DIR := build
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))


# Tests présents dans ce dossier
TEST_SRCS := $(wildcard ./*.c)
TEST_NAMES := $(patsubst %.c,%, $(notdir $(TEST_SRCS)))
TEST_OBJS := $(patsubst %, $(OBJ_DIR)/test_%.o, $(TEST_NAMES))

DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

LDFLAGS := $(SDL_LDFLAGS) -lm -Wl,-rpath,'$$ORIGIN/../../../client/SDL2/lib'

.PHONY: all clean run


all: $(patsubst %, $(OBJ_DIR)/%,$(TEST_NAMES))

# construire build/<name> pour chaque <name.c>
$(OBJ_DIR)/%: $(OBJ_DIR)/test_%.o $(OBJS) | $(OBJ_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile les .c du client
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

# Compile les fichiers de test locaux
$(OBJ_DIR)/test_%.o: ./%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Le binaire est placé dans $(OBJ_DIR) (TARGET = $(OBJ_DIR)/game)


clean:
	rm -rf $(OBJ_DIR)

-include $(DEPS)

run: all
	# Exécute build/game si présent
	if [ -x $(OBJ_DIR)/game ]; then $(OBJ_DIR)/game; else echo "No $(OBJ_DIR)/game executable"; fi

