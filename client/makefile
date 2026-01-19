# Paths
SDL_BUILD = ./SDL/build
SDL_INCLUDE = $(SDL_BUILD)/include
SDL_LIB = $(SDL_BUILD)/build/.libs

# Executable principal
client: index.o tcp.o
	gcc index.o tcp.o -o client -I./lib -I$(SDL_INCLUDE) -L$(SDL_LIB) -lSDL2 -lSDL2main -Wl,-rpath,$(SDL_LIB)
	make clean

# Bulding objets
index.o:
	gcc -c src/index.c -o index.o -I./lib -I$(SDL_INCLUDE)
tcp.o:
	gcc -c src/tcp.c -o tcp.o -I./lib -I$(SDL_INCLUDE)

# Clean objects
clean:
	rm -f *.o