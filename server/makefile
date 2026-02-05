# Build directory
DIR = build
# Create build directory if not exists
$(shell mkdir -p $(DIR))

# Executable principal
$(DIR)/server: $(DIR)/game.o $(DIR)/index.o $(DIR)/utils.o $(DIR)/tcp.o $(DIR)/lobby.o $(DIR)/user.o $(DIR)/message.o
	gcc $(DIR)/game.o $(DIR)/index.o $(DIR)/utils.o $(DIR)/tcp.o $(DIR)/lobby.o $(DIR)/user.o $(DIR)/message.o -o $(DIR)/server -I./lib

# Building objets
$(DIR)/index.o:
	gcc -c src/index.c -o $(DIR)/index.o -I./lib

$(DIR)/lobby.o:
	gcc -c src/lobby.c -o $(DIR)/lobby.o -I./lib

$(DIR)/message.o:
	gcc -c src/message.c -o $(DIR)/message.o -I./lib

$(DIR)/game.o:
	gcc -c src/game.c -o $(DIR)/game.o -I./lib

$(DIR)/user.o: src/user.c | $(DIR)
	gcc -c src/user.c -o $(DIR)/user.o -I./lib

$(DIR)/tcp.o:
	gcc -c src/tcp.c -o $(DIR)/tcp.o -I./lib

$(DIR)/utils.o:
	gcc -c src/utils.c -o $(DIR)/utils.o -I./lib


# Clean objects
clean:
	rm -rf $(DIR)