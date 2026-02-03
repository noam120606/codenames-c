# Executable principal
server: game.o index.o utils.o tcp.o lobby.o user.o
	gcc game.o index.o utils.o tcp.o lobby.o user.o -o server -I./lib

# Bulding objets
index.o:
	gcc -c src/index.c -o index.o -I./lib
lobby.o:
	gcc -c src/lobby.c -o lobby.o -I./lib
game.o:
	gcc -c src/game.c -o game.o -I./lib
user.o:
	gcc -c src/user.c -o user.o -I./lib
tcp.o:
	gcc -c src/tcp.c -o tcp.o -I./lib
utils.o:
	gcc -c src/utils.c -o utils.o -I./lib

# Clean objects
clean:
	rm -f *.o
	rm -rf server