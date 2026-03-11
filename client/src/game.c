#include "../lib/all.h"

SDL_Texture* card_h_classic;
SDL_Texture* card_f_classic;

int game_init(SDL_Context * context) {
    int loading_fails = 0;

    // Chargement image
    card_h_classic = load_image(context->renderer, "assets/img/cards/H_Classic.png");
    if (!card_h_classic) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_classic = load_image(context->renderer, "assets/img/cards/F_Classic.png");
    if (!card_f_classic) {
        printf("Failed to load card image\n");
        loading_fails++;
    }

    return loading_fails;
}

void game_display(SDL_Context * context) {

    if (context->game_state == GAME_STATE_PLAYING) {

        
        if (!audio_is_playing(MUSIC_MENU)) {
            audio_play(MUSIC_MENU, -1);
        }

        // Afficher les cartes du jeu
        int random_sexe = rand() % 2;
        int x = 20;
        int y = 25;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (random_sexe == 0) {
                    display_image(context->renderer, card_h_classic, x, y, 1.0, 0, SDL_FLIP_NONE, 1, 255);
                } else {
                    display_image(context->renderer, card_f_classic, x, y, 1.0, 0, SDL_FLIP_NONE, 1, 255);
                }
                x += 20;
            }
            y += 20;
        }

    }
}

int game_free() {
    
    return EXIT_SUCCESS;
}