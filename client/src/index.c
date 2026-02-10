#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../lib/tcp.h"
#include "../lib/sdl.h"
#include "../lib/menu.h"
#include "../lib/button.h"
#include "../lib/background.h"

// Bouton test
#define BTN_TEST 1
int on_test_button_click(SDL_Context context, int button_id) {
    printf("Bouton de test cliqué! (ID: %d)\n", button_id);
    return 0;
}

int main(int argc, char* argv[]){

    char ip[16] = "127.0.0.1";
    int port = 0;

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
    if (port == 0) {
        fprintf(stderr, "Port number is required. Usage: %s [-s server_ip] [-p port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Starting Codenames Client...\n");
    
    // Initialize TCP connection to server
    int sock = init_tcp(ip, port);
    if (sock < 0) {
        printf("Failed to connect to the server at %s:%d\n", ip, port);
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

    // Initialiser le système de boutons
    buttons_init();

    int menu_loading_fails = menu_init(context);
    if (menu_loading_fails > 0) {
        printf("Failed to load %d menu resource(s)\n", menu_loading_fails);
        close_tcp(sock);
        menu_free();
        destroy_context(context);
        return EXIT_FAILURE;
    }
    int background_loading_fails = init_background(context);
    if (background_loading_fails > 0) {
        printf("Failed to load %d background resource(s)\n", background_loading_fails);
        close_tcp(sock);
        menu_free();
        destroy_context(context);
        destroy_background();
        return EXIT_FAILURE;
    }

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(sock) == EXIT_SUCCESS) {

        // Gestion events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            // Gestion bouttons
            ButtonReturn result = buttons_handle_event(context, &e);
            switch (result) {
                case BTN_RET_QUIT: running = 0; break;
                default: break;
            }
        }

        // Pré Rendu
        SDL_SetRenderDrawColor(context.renderer, 0, 0, 0, 255);
        SDL_RenderClear(context.renderer);

        // Rendu et logique d'affichage
        display_background(context);
        menu_display(context);

        // Afficher les boutons
        buttons_display(context.renderer);

        // Post Rendu
        SDL_RenderPresent(context.renderer);
        SDL_Delay(16); // environ 60 rendus par seconde
        context.clock++;
    }

    printf("Exiting...\n");

    close_tcp(sock);
    menu_free();
    destroy_background();
    buttons_free();
    destroy_context(context);

    return EXIT_SUCCESS;
}