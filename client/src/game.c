#include "../lib/all.h"

SDL_Texture* card_h_classic;
SDL_Texture* card_f_classic;
SDL_Texture* card_h_red;
SDL_Texture* card_f_red;
SDL_Texture* card_h_blue;
SDL_Texture* card_f_blue;
SDL_Texture* card_black;

void game_handle_event(SDL_Context* context, SDL_Event* e) {
    // gestion evenements sdl
}

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
    card_h_red = load_image(context->renderer, "assets/img/cards/H_Red.png");
    if (!card_h_red) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_red = load_image(context->renderer, "assets/img/cards/F_Red.png");
    if (!card_f_red) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_h_blue = load_image(context->renderer, "assets/img/cards/H_Blue.png");
    if (!card_h_blue) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_blue = load_image(context->renderer, "assets/img/cards/F_Blue.png");
    if (!card_f_blue) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_black = load_image(context->renderer, "assets/img/cards/Black.png");
    if (!card_black) {
        printf("Failed to load card image\n");
        loading_fails++;
    }

    return loading_fails;
}

void game_render_cards(SDL_Context * context) {
    int x=0;
    int y=0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (!context->cards[i*5 + j]) {
                x += 50;
                continue;
            }
            if(context->player_role == ROLE_SPY && !context->cards[i*5 + j]->revealed) {
                switch (context->cards[i*5 + j]->team) {
                    case TEAM_NONE:
                        if (context->cards[i*5 + j]->gender) {
                            display_image(context->renderer, card_f_classic, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_classic, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_RED:
                        if (context->cards[i*5 + j]->gender) {
                            display_image(context->renderer, card_f_red, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_red, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_BLUE:
                        if (context->cards[i*5 + j]->gender) {
                            display_image(context->renderer, card_f_blue, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_blue, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_BLACK:
                        display_image(context->renderer, card_black, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                    default:
                        break;
                }
            }else if (context->player_role == ROLE_AGENT && !context->cards[i*5 + j]->revealed) {
                if (context->cards[i*5 + j]->gender) {
                    display_image(context->renderer, card_f_classic, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                }else{
                    display_image(context->renderer, card_h_classic, x, y, 0.05, 0, SDL_FLIP_NONE, 1, 255);
                }
            }
            else if (context->cards[i*5 + j]->revealed) {
                // Attente des images des cartes "révélées"
            }
            x += 250;
        }
        x = 0;
        y += 150;
    }
}

void game_display(SDL_Context * context) {

    if (!audio_is_playing(MUSIC_GAME)) {
        audio_play(MUSIC_GAME, -1);
    }
    game_render_cards(context);
    
}


int game_free() {
    if (card_h_classic) { free_image(card_h_classic); card_h_classic = NULL; }
    if (card_f_classic) { free_image(card_f_classic); card_f_classic = NULL; }
    if (card_h_red) { free_image(card_h_red); card_h_red = NULL; }
    if (card_f_red) { free_image(card_f_red); card_f_red = NULL; }
    if (card_h_blue) { free_image(card_h_blue); card_h_blue = NULL; }
    if (card_f_blue) { free_image(card_f_blue); card_f_blue = NULL; }
    if (card_black) { free_image(card_black); card_black = NULL; }
    return EXIT_SUCCESS;
}