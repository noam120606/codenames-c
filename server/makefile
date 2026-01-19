# Executable principal
server: game_manager.o index.o utils.o tcp.o
	gcc game_manager.o index.o utils.o tcp.o -o server -I./lib
	make clean

# Bulding objets
index.o:
	gcc -c src/index.c -o index.o -I./lib
game_manager.o:
	gcc -c src/game_manager.c -o game_manager.o -I./lib
tcp.o:
	gcc -c src/tcp.c -o tcp.o -I./lib
utils.o:
	gcc -c src/utils.c -o utils.o -I./lib

# Clean objects
clean:
	rm -f *.o