#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../lib/tcp.h"
#include "../lib/sdl.h"

const int PORT = 4242;
const char* SERVER_IP = "127.0.0.1";

int main(){

    printf("Starting Codenames Client...\n");
    
    // Initialize TCP connection to server
    int sock = init_tcp(SERVER_IP, PORT);
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

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);

    send_tcp(sock, "Hello from client!");

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(sock) == 0) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_Delay(16); 
    }

    printf("Exiting...\n");

    close_tcp(sock);
    destroy_context(context);
    return EXIT_SUCCESS;
} 

