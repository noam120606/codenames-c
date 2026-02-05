# Build directory
DIR = build
# Create build directory if not exists
$(shell mkdir -p $(DIR))

SDL_INCLUDE = ./SDL2/include/SDL2
SDL_LIB = ./SDL2/lib

CFLAGS = -I./lib -I$(SDL_INCLUDE)
LDFLAGS = -L$(SDL_LIB) -Wl,-rpath,$(SDL_LIB)
LDLIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2main

$(DIR)/client: $(DIR)/index.o $(DIR)/tcp.o
	gcc $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Building objects
$(DIR)/index.o:
	gcc -c src/index.c -o $@ $(CFLAGS)

$(DIR)/tcp.o:
	gcc -c src/tcp.c -o $@ $(CFLAGS)

clean:
	rm -rf build


