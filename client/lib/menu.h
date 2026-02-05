#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"

int menu_init(SDL_Context context);
int menu_display(SDL_Context context);
int menu_free(SDL_Context context);

#endif // MENU_H