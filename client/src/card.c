#include "../lib/all.h"

/* Images des cartes */
SDL_Texture* card_black;
SDL_Texture* card_black_revealed;
SDL_Texture* card_blue_h;
SDL_Texture* card_blue_f;
SDL_Texture* card_blue_h_revealed;
SDL_Texture* card_blue_f_revealed;
SDL_Texture* card_blue_c_revealed;
SDL_Texture* card_blue_d_revealed;
SDL_Texture* card_red_h;
SDL_Texture* card_red_f;
SDL_Texture* card_red_h_revealed;
SDL_Texture* card_red_f_revealed;
SDL_Texture* card_red_c_revealed;
SDL_Texture* card_red_d_revealed;
SDL_Texture* card_none_h;
SDL_Texture* card_none_f;
SDL_Texture* card_none_h_revealed;
SDL_Texture* card_none_f_revealed;

SDL_Texture* guess_icon;

/* Textes pour les mots des cartes (25 cartes) */
static Text* txt_card_words[NUM_CARDS] = {NULL};

int init_cards(AppContext * context) {

    int loading_fails = 0;

    // Chargement image
    card_black = load_image(context->renderer, "assets/img/cards/black/B_r217.png");
    if (!card_black) loading_fails++;
    card_black_revealed = load_image(context->renderer, "assets/img/cards/black/revealed/B_r217.png");
    if (!card_black_revealed) loading_fails++;
    card_blue_h = load_image(context->renderer, "assets/img/cards/blue/H_r217.png");
    if (!card_blue_h) loading_fails++;
    card_blue_f = load_image(context->renderer, "assets/img/cards/blue/F_r217.png");
    if (!card_blue_f) loading_fails++;
    card_blue_h_revealed = load_image(context->renderer, "assets/img/cards/blue/revealed/H_r217.png");
    if (!card_blue_h_revealed) loading_fails++;
    card_blue_f_revealed = load_image(context->renderer, "assets/img/cards/blue/revealed/F_r217.png");
    if (!card_blue_f_revealed) loading_fails++;
    card_blue_c_revealed = load_image(context->renderer, "assets/img/cards/blue/revealed/C_r217.png");
    if (!card_blue_c_revealed) loading_fails++;
    card_blue_d_revealed = load_image(context->renderer, "assets/img/cards/blue/revealed/D_r217.png");
    if (!card_blue_d_revealed) loading_fails++;
    card_red_h = load_image(context->renderer, "assets/img/cards/red/H_r217.png");
    if (!card_red_h) loading_fails++;
    card_red_f = load_image(context->renderer, "assets/img/cards/red/F_r217.png");
    if (!card_red_f) loading_fails++;
    card_red_h_revealed = load_image(context->renderer, "assets/img/cards/red/revealed/H_r217.png");
    if (!card_red_h_revealed) loading_fails++;
    card_red_f_revealed = load_image(context->renderer, "assets/img/cards/red/revealed/F_r217.png");
    if (!card_red_f_revealed) loading_fails++;
    card_red_c_revealed = load_image(context->renderer, "assets/img/cards/red/revealed/C_r217.png");
    if (!card_red_c_revealed) loading_fails++;
    card_red_d_revealed = load_image(context->renderer, "assets/img/cards/red/revealed/D_r217.png");
    if (!card_red_d_revealed) loading_fails++;
    card_none_h = load_image(context->renderer, "assets/img/cards/none/H_r217.png");
    if (!card_none_h) loading_fails++;
    card_none_f = load_image(context->renderer, "assets/img/cards/none/F_r217.png");
    if (!card_none_f) loading_fails++;
    card_none_h_revealed = load_image(context->renderer, "assets/img/cards/none/revealed/H_r217.png");
    if (!card_none_h_revealed) loading_fails++;
    card_none_f_revealed = load_image(context->renderer, "assets/img/cards/none/revealed/F_r217.png");
    if (!card_none_f_revealed) loading_fails++;

    guess_icon = load_image(context->renderer, "assets/img/cards/guess.png");
    if (!guess_icon) loading_fails++;

    /* Textes pour les mots des cartes */
    for (int i = 0; i < NUM_CARDS; i++) {
        txt_card_words[i] = init_text(context, " ", 
            create_text_config(FONT_BEBASKAI, 32, COL_BLACK, 0, 0, 0, 255));
    }

    return loading_fails;
}

static int card_handle_click(AppContext* context, Card* card, Booleen is_guess_icon) {
    if (!context || !card) return EXIT_FAILURE;

    if (card->selected && is_guess_icon) {
        printf("Card \"%s\" guessed!\n", card->word);

        char message[64];
        const char* agent_name = (context->player_name && context->player_name[0] != '\0') ? context->player_name : "Unknown";
        format_to(message, sizeof(message), "%d %d %s", MSG_GUESS_CARD, (int)(card - context->lobby->game->cards), agent_name);
        if (send_tcp(context->sock, message) != EXIT_SUCCESS) {
            printf("Failed to send click_card message to server\n");
            return EXIT_FAILURE;
        }
        
        card->selected = False;
        card->revealed = True;
    } else {
        card->selected = !card->selected;
        char msg[64];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_PREGUESS, (int)(card - context->lobby->game->cards), card->selected);
        send_tcp(context->sock, msg);
    } 

    return EXIT_SUCCESS;
}

static int is_mouse_over_card(Card* card, int mouseX, int mouseY) {
    return mouseX >= card->rect.x && mouseX <= card->rect.x + card->rect.w && mouseY >= card->rect.y && mouseY <= card->rect.y + card->rect.h;
}

static int is_mouse_over_guess_icon(Card* card, int mouseX, int mouseY) {
    return mouseX >= card->guess_rect.x && mouseX <= card->guess_rect.x + card->guess_rect.w && mouseY >= card->guess_rect.y && mouseY <= card->guess_rect.y + card->guess_rect.h;
}

static int card_handle_event(AppContext* context, SDL_Event* event, Card* card) {
    if (!context || !event || !card) return EXIT_FAILURE;

    if (event->type == SDL_MOUSEMOTION) {
        int mouseX = event->motion.x;
        int mouseY = event->motion.y;
        card->is_hovered = is_mouse_over_card(card, mouseX, mouseY);
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mouseX = event->button.x;
        int mouseY = event->button.y;
        int is_over = is_mouse_over_card(card, mouseX, mouseY);
        card->is_hovered = is_over;
        card->is_pressed = is_over;
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        int mouseX = event->button.x;
        int mouseY = event->button.y;
        int is_over = is_mouse_over_card(card, mouseX, mouseY);
        int should_trigger = card->is_pressed && is_over;

        card->is_hovered = is_over;
        card->is_pressed = False;

        if (should_trigger) {
            audio_play(SOUND_BUTTON_CLICKED, 0);
            return card_handle_click(context, card, is_mouse_over_guess_icon(card, mouseX, mouseY));
        }
    }
    return EXIT_SUCCESS;
}

int cards_handle_event(AppContext* context, SDL_Event* event) {
    if (!context || !event || !context->lobby || !context->lobby->game) return EXIT_FAILURE;

    for (int i = 0; i < NUM_CARDS; i++) {
        if (context->player_role == ROLE_AGENT && my_turn(context) && !context->lobby->game->cards[i].revealed) {
            card_handle_event(context, event, context->lobby->game->cards + i);
        } 
    }

    return EXIT_SUCCESS;
}

static SDL_Texture* get_card_texture(AppContext* context, Card* card) {
    if (card->revealed) {
        switch (card->team) {
            case TEAM_NONE:
                switch (card->type) {
                    case CT_MALE: case CT_CAT: return card_none_h_revealed; break;
                    case CT_FEMALE: case CT_DOG: return card_none_f_revealed; break;
                    default: return NULL;
                } break;
            case TEAM_RED:
                switch (card->type) {
                    case CT_MALE: return card_red_h_revealed; break;
                    case CT_FEMALE: return card_red_f_revealed; break;
                    case CT_CAT: return card_red_c_revealed; break;
                    case CT_DOG: return card_red_d_revealed; break;
                    default: return NULL;
                } break;
            case TEAM_BLUE:
                switch (card->type) {
                    case CT_MALE: return card_blue_h_revealed; break;
                    case CT_FEMALE: return card_blue_f_revealed; break;
                    case CT_CAT: return card_blue_c_revealed; break;
                    case CT_DOG: return card_blue_d_revealed; break;
                    default: return NULL;
                } break;
            case TEAM_BLACK: return card_black_revealed; break;
            default: return NULL;
        }
    } else {
        if (context->player_role == ROLE_SPY || context->lobby->game->state == GAMESTATE_ENDED) {
            switch (card->team) {
                case TEAM_NONE: return card->type % 2 ? card_none_f : card_none_h; break;
                case TEAM_RED: return card->type % 2 ? card_red_f : card_red_h; break;
                case TEAM_BLUE: return card->type % 2 ? card_blue_f : card_blue_h; break;
                case TEAM_BLACK: return card_black; break;
                default: return NULL;
            }
        } else {
            return card->type % 2 ? card_none_f : card_none_h;
        }
    }
} 

static int card_render(AppContext* context, Card* card, int x, int y, int index) {
    if (!context || !card) return EXIT_FAILURE;

    const int CARD_WIDTH = 200;
    const int CARD_HEIGHT = 128;

    card->rect = (SDL_Rect){
        .x = (WIN_WIDTH - CARD_WIDTH) / 2 + x,
        .y = (WIN_HEIGHT - CARD_HEIGHT) / 2 - y,
        .w = CARD_WIDTH,
        .h = CARD_HEIGHT
    };

    if (card->is_pressed && card->is_hovered) {
        card->rect.x += 2;
        card->rect.y += 2;
        card->rect.w -= 4;
        card->rect.h -= 2;
    } else if (card->is_hovered) {
        card->rect.x -= 4;
        card->rect.y -= 2;
        card->rect.w += 8;
        card->rect.h += 4;
    }
    
    SDL_RenderCopyEx(context->renderer, get_card_texture(context, card), NULL, &card->rect, 0, NULL, SDL_FLIP_NONE);

    if (card->selected) {
        const int GUESS_ICON_SIZE = 32;
        card->guess_rect = (SDL_Rect){
            .x = card->rect.x + card->rect.w - GUESS_ICON_SIZE,
            .y = card->rect.y,
            .w = GUESS_ICON_SIZE,
            .h = GUESS_ICON_SIZE
        };
        SDL_RenderCopyEx(context->renderer, guess_icon, NULL, &card->guess_rect, 0, NULL, SDL_FLIP_NONE);
    }

    if (!card->revealed && txt_card_words[index]) {
        update_text(context, txt_card_words[index], card->word);
        update_text_position(txt_card_words[index], x, y - 30);
        display_text(context, txt_card_words[index]);
    }

    return EXIT_SUCCESS;
}

void game_render_cards(AppContext * context) {
    int x=-420;
    int y=-235;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int card_index = i*5 + j;
            card_render(context, context->lobby->game->cards + card_index, x, y, card_index);
            x += 210;
        }
        x = -420;
        y += 128 + 10;
    }
}

int card_free() {
    if (card_black) { free_image(card_black); card_black = NULL; }
    if (card_black_revealed) { free_image(card_black_revealed); card_black_revealed = NULL; }
    if (card_blue_h) { free_image(card_blue_h); card_blue_h = NULL; }
    if (card_blue_f) { free_image(card_blue_f); card_blue_f = NULL; }
    if (card_blue_h_revealed) { free_image(card_blue_h_revealed); card_blue_h_revealed = NULL; }
    if (card_blue_f_revealed) { free_image(card_blue_f_revealed); card_blue_f_revealed = NULL; }
    if (card_blue_c_revealed) { free_image(card_blue_c_revealed); card_blue_c_revealed = NULL; }
    if (card_blue_d_revealed) { free_image(card_blue_d_revealed); card_blue_d_revealed = NULL; }
    if (card_red_h) { free_image(card_red_h); card_red_h = NULL; }
    if (card_red_f) { free_image(card_red_f); card_red_f = NULL; }
    if (card_red_h_revealed) { free_image(card_red_h_revealed); card_red_h_revealed = NULL; }
    if (card_red_f_revealed) { free_image(card_red_f_revealed); card_red_f_revealed = NULL; }
    if (card_red_c_revealed) { free_image(card_red_c_revealed); card_red_c_revealed = NULL; }
    if (card_red_d_revealed) { free_image(card_red_d_revealed); card_red_d_revealed = NULL; }
    if (card_none_h) { free_image(card_none_h); card_none_h = NULL; }
    if (card_none_f) { free_image(card_none_f); card_none_f = NULL; }
    if (card_none_h_revealed) { free_image(card_none_h_revealed); card_none_h_revealed = NULL; }
    if (card_none_f_revealed) { free_image(card_none_f_revealed); card_none_f_revealed = NULL; }

    if (guess_icon) { free_image(guess_icon); guess_icon = NULL; }

    for (int i = 0; i < NUM_CARDS; i++) {
        destroy_text(txt_card_words[i]); txt_card_words[i] = NULL;
    }

    return 0;
}