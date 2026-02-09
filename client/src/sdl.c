#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/tcp.h"
#include "../lib/sdl.h"



SDL_Context init_sdl() {

    SDL_Context context;

    // Initialize SDL
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return context;
    }

    // Create SDL window
    printf("Creating SDL window...\n");
    SDL_Window* win = SDL_CreateWindow("Codenames Client", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return context;
    }

    context.window = win;

    // Create SDL renderer
    printf("Creating SDL renderer...\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return context;
    }

    context.renderer = renderer;

    // Initialize IMG
    printf("Initializing IMG...\n");
    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return context;
    }   

    printf("All initialized successfully!\n");

    return context;
}

SDL_Texture* load_image(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* image = IMG_Load(path);
    if (!image) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture) {
        printf("Erreur de création de la texture : %s\n", SDL_GetError());
        return NULL;
    }
    return texture;
}

int display_image(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int w, int h) {
    if (!renderer || !texture) {
        return EXIT_FAILURE;
    }

    int tex_w = 0, tex_h = 0;
    if (SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h) != 0) {
        printf("SDL_QueryTexture Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Rect dstrect;
    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = (w > 0) ? w : tex_w;
    dstrect.h = (h > 0) ? h : tex_h;

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    return EXIT_SUCCESS;
}

void free_image(SDL_Texture* texture) {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

void destroy_context(SDL_Context context) {
    if (context.renderer) {
        SDL_DestroyRenderer(context.renderer);
    }
    if (context.window) {
        SDL_DestroyWindow(context.window);
    }
    IMG_Quit();
    SDL_Quit();
}