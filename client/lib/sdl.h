#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#ifndef INIT_H
#define INIT_H

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} SDL_Context;

SDL_Context init_sdl();
SDL_Surface* load_image(const char* path);
int display_image(SDL_Renderer* renderer, SDL_Surface* image);
void free_image(SDL_Surface* image);
void destroy_context(SDL_Context context);

#endif // INIT_H