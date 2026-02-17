#include "../lib/all.h"

int main(int argc, char *argv[]) {

    int port = 0;
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (port == 0) {
        fprintf(stderr, "Port number is required. Usage: %s [-p port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Starting the game server on port %d...\n", port);

    // Initialisations diverses
    srand(time(NULL));
    init_game_manager();
    init_lobby_manager();

    Codenames* codenames = malloc(sizeof(Codenames));
    if (codenames == NULL) {
        perror("Failed to create Codenames");
        return EXIT_FAILURE;
    }

    codenames->tcp = tcp_server_create(port);
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
    destroy_lobby_manager(codenames, codenames->lobby);
    tcp_server_destroy(codenames->tcp);
    free(codenames);

    printf("Server shutting down.\n");

    return EXIT_SUCCESS;
}