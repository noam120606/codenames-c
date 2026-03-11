#include "../lib/all.h"

void game_display(SDL_Context * context) {

    int in_lobby = context->lobby_id != -1;

    if (!audio_is_playing(MUSIC_MENU)) {
        audio_play(MUSIC_MENU, -1);
    }

    if (in_lobby) {
        hide_button(BTN_CREATE);
        hide_button(BTN_JOIN);

        char msg[64];
        format_to(msg, sizeof(msg), "Bienvenue %s ! Tu es lobby %d. Code : %s", name ? name : "invité", context->lobby_id, context->lobby_code ? context->lobby_code : "AUCUN");
        int desired_screen_y = 700;
        int rel_x = 0; // 0 = centré horizontalement
        int rel_y = (WIN_HEIGHT/2) - desired_screen_y; // négatif si desired_screen_y > WIN_HEIGHT/2
        text_display(context->renderer, msg, FONT_LARABIE, 24, COL_WHITE, rel_x, rel_y, 0, 255);

    } else if (joining) {
        show_button(BTN_CREATE);
        hide_button(BTN_JOIN);
        if (code_input) input_render(context->renderer, code_input);
    } else {
        show_button(BTN_CREATE);
        show_button(BTN_JOIN);
    }

    // Afficher le logo à sa taille d'origine
    if (menu_logo) {
        display_image(context->renderer, menu_logo, 0, 200, 1.00, 0, SDL_FLIP_NONE, 1, 255);
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