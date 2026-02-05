#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
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
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    // Create SDL window
    printf("Creating SDL window...\n");
    SDL_Window* win = SDL_CreateWindow("Codenames Client", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    // Create SDL renderer
    printf("Creating SDL renderer...\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    // Initialize IMG
    printf("Initializing IMG...\n");
    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }   

    // Load image
    printf("Loading image...\n");
    SDL_Surface* image = IMG_Load("assets/loup.jpg");
    if(!image)
    {
        printf("Erreur de chargement de l'image : %s",SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        IMG_Quit();
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    // Create texture from surface
    printf("Creating texture from surface...\n");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    printf("Texture size: %dx%d\n", w, h);
    SDL_FreeSurface(image);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        IMG_Quit();
        SDL_Quit();
        close_tcp(sock);
        return EXIT_FAILURE;
    }

    send_tcp(sock, "Hello from client!");

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(sock) == 0) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); 
    }

    printf("Exiting...\n");

    close_tcp(sock);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
} 

