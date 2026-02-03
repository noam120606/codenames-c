#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../lib/tcp.h"
#include "../lib/codenames.h"

const int PORT = 4242;

int main(int argc, char *argv[]) {

    printf("Starting the game server...\n");
    srand(time(NULL));

    // Initialize Codenames
    Codenames* codenames = malloc(sizeof(Codenames));
    if (codenames == NULL) {
        perror("Failed to create Codenames");
        return EXIT_FAILURE;
    }

    // Initialize TCP server
    codenames->tcp = tcp_server_create(PORT);
    if (codenames->tcp == NULL) {
        free(codenames);
        perror("Failed to create TCP server");
        return EXIT_FAILURE;
    }


    while(1) {
        tcp_server_tick(codenames);
    }

    // Cleanup
    tcp_server_destroy(codenames->tcp);
    free(codenames);
    printf("Server shutting down.\n");
    return EXIT_SUCCESS;
}