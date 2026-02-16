#include "../lib/all.h"

SDL_Texture* menu_logo;
SDL_Texture* quagmire;
Input* name_input = NULL;

static void name_on_submit(const char* text) {
    printf("Name input submitted: %s\n", text);
}

void menu_handle_event(SDL_Context* context, SDL_Event* e) {
    if (name_input) input_handle_event(name_input, e);
}

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

    // Chargement input
    const char* placeholders[] = {"Dédé", "Doudou", "Toto"};
    name_input = input_create(INPUT_ID_NAME, WIN_WIDTH/2 + 650, 50, 250, 60, "assets/fonts/larabiefont.otf", 28, placeholders, 3, "Pseudo : ", 128);
    if (name_input) {
        input_set_on_submit(name_input, name_on_submit);
        input_set_bg(name_input, context->renderer, "assets/img/inputs/empty.png", 14);
    }
    
    return loading_fails;
}

void menu_display(SDL_Context * context) {
    // Afficher le logo à sa taille d'origine
    if (menu_logo) {
        display_image(context->renderer, menu_logo, 0, 200, 0.75, 0, SDL_FLIP_NONE, 1, 255);
    }
    
    // Afficher quagmire à sa taille d'origine
    if (quagmire) {
        display_image(context->renderer, quagmire, 850, -350, 1.0, 0, SDL_FLIP_NONE, 1, 255);
    }

    /* input is drawn by menu */
    if (name_input) input_render(context->renderer, name_input);

    /* submitted text is drawn by input_render */
}

int menu_free() {
    if (menu_logo) free_image(menu_logo);
    if (quagmire) free_image(quagmire);
    if (name_input) input_destroy(name_input);

    return EXIT_SUCCESS;
}