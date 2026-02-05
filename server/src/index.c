#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../lib/tcp.h"
#include "../lib/game.h"
#include "../lib/lobby.h"
#include "../lib/codenames.h"

const int PORT = 4242;

int main(int argc, char *argv[]) {

    printf("Starting the game server...\n");

    // Initialisations diverses
    srand(time(NULL));
    init_game_manager();

    Codenames* codenames = malloc(sizeof(Codenames));
    if (codenames == NULL) {
        perror("Failed to create Codenames");
        return EXIT_FAILURE;
    }

    codenames->tcp = tcp_server_create(PORT);
    if (codenames->tcp == NULL) {
        free(codenames);
        perror("Failed to create TCP server");
        return EXIT_FAILURE;
    }

    codenames->lobby = create_lobby_manager();
    if (codenames->lobby == NULL) {
        tcp_server_destroy(codenames->tcp);
        free(codenames);
        perror("Failed to create LobbyManager");
        return EXIT_FAILURE;
    }

    // Boucle d'execution
    while(1) {
        tcp_server_tick(codenames);
    }

    // Cleanup
    tcp_server_destroy(codenames->tcp);
    destroy_lobby_manager(codenames->lobby);
    free(codenames);
    printf("Server shutting down.\n");
    return EXIT_SUCCESS;
}