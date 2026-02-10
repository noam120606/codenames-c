#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"
#include "../lib/background.h"
#include "../lib/button.h"

SDL_Texture* lens;

int init_background(SDL_Context context) {
    int loading_fails = 0;
    lens = load_image(context.renderer, "assets/img/background/lens.png");
    if (!lens) {
        printf("Failed to load lens image\n");
        loading_fails++;
    }

    return loading_fails;
}

void display_background(SDL_Context context) {
    // Display the background for each symbol
    int symbol_width = 32;
    int symbol_height = 32;

    int space_x = 16;
    int space_y = 32;

    for(int i= -4; i < WIN_WIDTH/symbol_width + 4; i++){
        for(int j = -4; j < WIN_HEIGHT/symbol_height + 4; j++){
            ////////////////////////////////////// NE PAS TOUCHER ////////////////////////////////////////////////////////
            int x = i * (symbol_width + space_x) - ((context.clock%(4 * (symbol_width + space_x)) - space_x*2)/2);
            int y = j * (symbol_height + space_y) + context.clock%(2 * (symbol_height + space_y)) + (i%2?space_y:0);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////
            display_image(context.renderer, lens, x, y, symbol_width, symbol_height);
        }
    }
}

void destroy_background() {
    if (lens) free_image(lens);
}