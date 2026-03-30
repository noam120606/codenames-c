#include "../lib/all.h"

/* Images des cartes */
SDL_Texture* card_h_classic;
SDL_Texture* card_f_classic;
SDL_Texture* card_h_red;
SDL_Texture* card_f_red;
SDL_Texture* card_h_blue;
SDL_Texture* card_f_blue;
SDL_Texture* card_black;

/* Textes pour les mots des cartes (25 cartes) */
static Text* txt_card_words[NUM_CARDS] = {NULL};

int init_cards(AppContext * context) {

    int loading_fails = 0;

    // Chargement image
    card_h_classic = load_image(context->renderer, "assets/img/cards/none/H_r217.png");
    if (!card_h_classic) loading_fails++;

    card_f_classic = load_image(context->renderer, "assets/img/cards/none/F_r217.png");
    if (!card_f_classic) loading_fails++;

    card_h_red = load_image(context->renderer, "assets/img/cards/red/H_r217.png");
    if (!card_h_red) loading_fails++;

    card_f_red = load_image(context->renderer, "assets/img/cards/red/F_r217.png");
    if (!card_f_red) loading_fails++;

    card_h_blue = load_image(context->renderer, "assets/img/cards/blue/H_r217.png");
    if (!card_h_blue) loading_fails++;

    card_f_blue = load_image(context->renderer, "assets/img/cards/blue/F_r217.png");
    if (!card_f_blue) loading_fails++;

    card_black = load_image(context->renderer, "assets/img/cards/black/B_r217.png");
    if (!card_black) loading_fails++;

    /* Textes pour les mots des cartes */
    for (int i = 0; i < NUM_CARDS; i++) {
        txt_card_words[i] = init_text(context, " ", 
            create_text_config(FONT_BEBASKAI, 28, COL_BLACK, 0, 0, 0, 255));
    }

    return loading_fails;
}

int card_render(AppContext* context, Card* cards) {
    if (!context || !cards) return -1;

    // Rendu de la carte en fonction de son état
    // if (cards->is_hovered) {
    //     // Afficher la carte survolée
    // } else if (cards->is_pressed) {
    //     // Afficher la carte pressée
    // } else {
    //     // Afficher la carte par défaut
    // }

    return 0;
}

void game_render_cards(AppContext * context) {
    int x=-400;
    int y=-250;
    int card_index = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Card card = context->lobby->game->cards[i*5 + j];

            if(context->player_role == ROLE_SPY && !card.revealed) {
                switch (card.team) {
                    case TEAM_NONE:
                        if (card.type) {
                            display_image(context->renderer, card_f_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_RED:
                        if (card.type) {
                            display_image(context->renderer, card_f_red, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_red, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_BLUE:
                        if (card.type) {
                            display_image(context->renderer, card_f_blue, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_blue, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_BLACK:
                        display_image(context->renderer, card_black, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                    default:
                        break;
                }
            }else if (context->player_role == ROLE_AGENT && !card.revealed) {
                if (card.type) {
                    display_image(context->renderer, card_f_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                }else{
                    display_image(context->renderer, card_h_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                }
            }
            else if (card.revealed) {
                // Attente des images des cartes "révélées"
            }
            
            /* Affichage du mot de la carte avec le nouveau système */
            if (card_index < NUM_CARDS && txt_card_words[card_index]) {
                update_text(context, txt_card_words[card_index], card.word);
                update_text_position(txt_card_words[card_index], x, y - 22);
                display_text(context, txt_card_words[card_index]);
            }
            card_index++;
            x += 200;
        }
        x = -400;
        y += 125;
    }
}

int card_free() {
    if (card_h_classic) { free_image(card_h_classic); card_h_classic = NULL; }
    if (card_f_classic) { free_image(card_f_classic); card_f_classic = NULL; }
    if (card_h_red) { free_image(card_h_red); card_h_red = NULL; }
    if (card_f_red) { free_image(card_f_red); card_f_red = NULL; }
    if (card_h_blue) { free_image(card_h_blue); card_h_blue = NULL; }
    if (card_f_blue) { free_image(card_f_blue); card_f_blue = NULL; }
    if (card_black) { free_image(card_black); card_black = NULL; }

    for (int i = 0; i < NUM_CARDS; i++) {
        destroy_text(txt_card_words[i]); txt_card_words[i] = NULL;
    }
    return 0;
}