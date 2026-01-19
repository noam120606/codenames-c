#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "tcp.h"

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

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    printf("SDL Initialized successfully\n");

    SDL_Window* win = SDL_CreateWindow("Codenames Client", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    printf("Window created successfully\n");

    send_tcp(sock, "Hello from client!");

    while (tick_tcp(sock) == 0) {
        printf("Tick...\n");

    }

    printf("Exiting...\n");

    close_tcp(sock);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return EXIT_SUCCESS;
} 

