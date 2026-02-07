#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../lib/tcp.h"
#include "../lib/sdl.h"
#include "../lib/menu.h"

int main(int argc, char* argv[]){

    char ip[16] = "127.0.0.1";
    int port = 4242;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
            case 's':
                strncpy(ip, optarg, sizeof(ip) - 1);
                ip[sizeof(ip) - 1] = '\0';
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-s server_ip] [-p port]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    printf("Starting Codenames Client...\n");
    
    // Initialize TCP connection to server
    int sock = init_tcp(ip, port);
    if (sock < 0) {
        printf("Failed to connect to the server\n");
        return EXIT_FAILURE;
    }

    SDL_Context context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        close_tcp(sock);
        destroy_context(context);
        return EXIT_FAILURE;
    }

    printf("Connected to server at %s:%d\n", ip, port);

    menu_init(context);

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(sock) == 0) {

        // Gestion events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Pré Rendu
        SDL_SetRenderDrawColor(context.renderer, 0, 0, 0, 255);
        SDL_RenderClear(context.renderer);

        // Rendu et logique d'affichage
        int action = menu_display(context);

        // Post Rendu
        SDL_RenderPresent(context.renderer);
        SDL_Delay(16);
    }

    printf("Exiting...\n");

    close_tcp(sock);
    menu_free(context);
    destroy_context(context);

    return EXIT_SUCCESS;
} 

