#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"
#include "../lib/menu.h"
#include "../lib/button.h"

SDL_Texture* menu_logo;
SDL_Texture* quagmire;

ButtonReturn menu_button_click(SDL_Context context, ButtonId button_id) {
    switch (button_id) {
        case BTN_START: printf("Start button clicked\n"); break;
        case BTN_QUIT: return BTN_RET_QUIT; break;
        default: printf("Unknown button clicked\n"); break;
    }
    return BTN_RET_NONE;
}

int menu_init(SDL_Context context) {
    int loading_fails = 0;

    // Chargement image
    menu_logo = load_image(context.renderer, "assets/img/placeholder/logo.png");
    if (!menu_logo) {
        printf("Failed to load menu logo image\n");
        loading_fails++;
    }
    quagmire = load_image(context.renderer, "assets/img/others/quagmire.png");
    if (!quagmire) {
        printf("Failed to load quagmire image\n");
        loading_fails++;
    }

    // Chargement bouton
    SDL_Texture* img_btn_start = load_image(context.renderer, "assets/img/buttons/start.png");
    if (!img_btn_start) {
        printf("Failed to load start button image\n");
        loading_fails++;
    }
    SDL_Texture* img_btn_quit = load_image(context.renderer, "assets/img/buttons/quit.png");
    if (!img_btn_quit) {
        printf("Failed to load quit button image\n");
        loading_fails++;
    }
    
    if (img_btn_start) {
        button_create(BTN_START, WIN_WIDTH/2-250, 550, 500, 125, img_btn_start, menu_button_click);
    }
    if (img_btn_quit) {
        button_create(BTN_QUIT, WIN_WIDTH/2-150, 800, 300, 75, img_btn_quit, menu_button_click);
    }

    return loading_fails;
}

void menu_display(SDL_Context context) {
    if (menu_logo) display_image(context.renderer, menu_logo, (WIN_WIDTH/2)-512, 128, 1024, 0);
    if (quagmire) display_image(context.renderer, quagmire, WIN_WIDTH-244, WIN_HEIGHT-357, 0, 0);
}

int menu_free() {
    if (menu_logo) free_image(menu_logo);
    if (quagmire) free_image(quagmire);

    return EXIT_SUCCESS;
}