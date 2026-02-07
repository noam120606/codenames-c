#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"
#include "../lib/menu.h"

SDL_Texture* img_menu;
int temp = 0;

int menu_init(SDL_Context context) {
    int loading_fails = 0;

    img_menu = load_image(context.renderer, "assets/quagmire.png");
    if (!img_menu) {
        printf("Failed to load menu image\n");
        loading_fails++;
    }
    
    return loading_fails;
}

MenuAction menu_display(SDL_Context context) {
    if (img_menu) {
        display_image(context.renderer, img_menu, temp++, 0, 0, 0);
    }
    return MENU_ACTION_NONE;
}

int menu_free(SDL_Context context) {
    if (img_menu) {
        free_image(img_menu);
        img_menu = NULL;
    }
    return EXIT_SUCCESS;
}