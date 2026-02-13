# Build directory
DIR = build
# Create build directory if not exists
$(shell mkdir -p $(DIR))

# Use system SDL2 via pkg-config
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf 2>/dev/null || echo "-I/usr/include/SDL2")
SDL_LDFLAGS := $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf 2>/dev/null || echo "-lSDL2 -lSDL2_image -lSDL2_ttf")

CFLAGS = -I./lib $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS) -lm


$(DIR)/client: $(DIR)/index.o $(DIR)/tcp.o $(DIR)/sdl.o $(DIR)/menu.o $(DIR)/button.o $(DIR)/background.o $(DIR)/infos.o $(DIR)/texts.o
	gcc $^ -o $@ $(LDFLAGS)
$(DIR)/index.o:
	gcc -c src/index.c -o $@ $(CFLAGS)
$(DIR)/tcp.o:
	gcc -c src/tcp.c -o $@ $(CFLAGS)
$(DIR)/sdl.o:
	gcc -c src/sdl.c -o $@ $(CFLAGS)
$(DIR)/menu.o:
	gcc -c src/menu.c -o $@ $(CFLAGS)
$(DIR)/button.o:
	gcc -c src/button.c -o $@ $(CFLAGS)
$(DIR)/background.o:
	gcc -c src/background.c -o $@ $(CFLAGS)
$(DIR)/infos.o:
	gcc -c src/infos.c -o $@ $(CFLAGS)
$(DIR)/texts.o:
	gcc -c src/texts.c -o $@ $(CFLAGS)

clean:
	rm -rf build