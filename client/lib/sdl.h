#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} SDL_Context;

// Fonctions
SDL_Context init_sdl();
SDL_Texture* load_image(SDL_Renderer* renderer, const char* path);
int display_image(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int w, int h);
void free_image(SDL_Texture* texture);
void destroy_context(SDL_Context context);

#endif // SDL_H