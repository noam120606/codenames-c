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
    printf("Button clicked: %d\n", button_id);
    switch (button_id) {
        case BTN_JOIN: printf("Join button clicked\n"); break;
        case BTN_CREATE: printf("Create button clicked\n"); break;
        case BTN_QUIT: return BTN_RET_QUIT; break;
        default: printf("Unknown button clicked\n"); break;
    }
    return BTN_RET_NONE;
}

int menu_init(SDL_Context * context) {
    int loading_fails = 0;

    // Chargement image
    menu_logo = load_image(context->renderer, "assets/img/others/logo.png");
    if (!menu_logo) {
        printf("Failed to load menu logo image\n");
        loading_fails++;
    }
    quagmire = load_image(context->renderer, "assets/img/others/quagmire.png");
    if (!quagmire) {
        printf("Failed to load quagmire image\n");
        loading_fails++;
    }

    // Chargement bouton
    text_button_create(context->renderer, BTN_CREATE, WIN_WIDTH/2-450, 700, 100, "Créer", "assets/fonts/larabiefont.otf", (SDL_Color){255, 255, 255, 255}, menu_button_click);
    text_button_create(context->renderer, BTN_JOIN, WIN_WIDTH/2+50, 700, 100, "Rejoindre", "assets/fonts/larabiefont.otf", (SDL_Color){255, 255, 255, 255}, menu_button_click);
    text_button_create(context->renderer, BTN_QUIT, WIN_WIDTH/2-200, 900, 100, "Quitter", "assets/fonts/larabiefont.otf", (SDL_Color){255, 255, 255, 255}, menu_button_click);
    
    return loading_fails;
}

void menu_display(SDL_Context * context) {
    if (menu_logo) display_image(context->renderer, menu_logo, (WIN_WIDTH/2)-512, -32, 1024, 0);
    if (quagmire) display_image(context->renderer, quagmire, WIN_WIDTH-244, WIN_HEIGHT-357, 0, 0);
}

int menu_free() {
    if (menu_logo) free_image(menu_logo);
    if (quagmire) free_image(quagmire);

    return EXIT_SUCCESS;
}