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

TEST_SRC := game.c
TEST_OBJ := $(OBJ_DIR)/test_game.o

TARGET := ./game

DEPS := $(OBJS:.o=.d) $(TEST_OBJ:.o=.d)

LDFLAGS := $(SDL_LDFLAGS) -lm -Wl,-rpath,'$$ORIGIN/../../../client/SDL2/lib'

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) $(TEST_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile les .c du client
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

# Compile le fichier de test local
$(TEST_OBJ): $(TEST_SRC) | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Le binaire est placé à la racine du dossier tests/client (TARGET = ./game)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

-include $(DEPS)

run: all
	# Exécute ./game depuis tests/client ; rpath est défini pour trouver les libs SDL embarquées.
	./game

