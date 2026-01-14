#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../include/game_manager.h"
#include "../include/tcp.h"

int main(int argc, char *argv[]) {

    printf("Starting the game server...\n");
    srand(time(NULL));

    // Initialize game manager
    GameManager* game_manager = create_game_manager();
    if (game_manager == NULL) {
        perror("Failed to create game manager");
        return EXIT_FAILURE;
    }

    // Initialize TCP server
    TcpServer* tcp = tcp_server_create(4242);
    if (tcp == NULL) {
        perror("Failed to create TCP server");
        destroy_game_manager(game_manager);
        return EXIT_FAILURE;
    }

    /* Debut zone de tests */

    Lobby* lobby = new_lobby(game_manager, 4);
    if (lobby == NULL) {
        perror("Failed to create new lobby");
        destroy_game_manager(game_manager);
        return EXIT_FAILURE;
    }

    new_game(game_manager, lobby->lobby_id);
    print_game(&lobby->game);
    char* json = game_to_json(&lobby->game);
    if (json) {
        printf("Game JSON: %s\n", json);
        free(json);
    }

    /* Fin zone de tests */

    while(1) {
        tcp_server_tick(tcp);
    }

    // Cleanup
    destroy_game_manager(game_manager);
    tcp_server_destroy(tcp);
    printf("Server shutting down.\n");
    return EXIT_SUCCESS;
}