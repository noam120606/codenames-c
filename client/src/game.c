#include "../lib/all.h"

SDL_Texture* card_h_classic;
SDL_Texture* card_f_classic;
SDL_Texture* card_h_red;
SDL_Texture* card_f_red;
SDL_Texture* card_h_blue;
SDL_Texture* card_f_blue;
SDL_Texture* card_black;
Button* btn_quit_game = NULL;

/* Utilisation des icônes déjà chargées dans lobby.c */
extern SDL_Texture* player_icon_red;
extern SDL_Texture* player_icon_blue;

/* Couleurs pour les panneaux d'équipe */
static const SDL_Color TEAM_BLUE_COLOR = {50, 80, 150, 200};
static const SDL_Color TEAM_RED_COLOR = {150, 50, 50, 200};
static const SDL_Color PANEL_BG_COLOR = {30, 30, 30, 180};

static ButtonReturn game_button_click(AppContext* context, Button* button) {
    if (!context || !button) return BTN_RET_NONE;

    if (button == btn_quit_game) {
        context->app_state = APP_STATE_MENU;

        struct_lobby_init(context->lobby, -1, "");
        char msg[16];
        format_to(msg, sizeof(msg), "%d", MSG_LEAVELOBBY);
        send_tcp(context->sock, msg);

        printf("Left game and returned to menu\n");
    }

    return BTN_RET_NONE;
}

void game_handle_event(AppContext* context, SDL_Event* e) {
    if (!context || !e) return;
    if (btn_quit_game) {
        button_handle_event(context, btn_quit_game, e);
    }
}

int game_init(AppContext * context) {

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

    ButtonConfig* cfg_btn_quit_game = button_config_init();
    if (cfg_btn_quit_game) {
        cfg_btn_quit_game->x         = -730;
        cfg_btn_quit_game->y         = -450;
        cfg_btn_quit_game->h         = 64;
        cfg_btn_quit_game->font_path = FONT_LARABIE;
        cfg_btn_quit_game->color     = COL_WHITE;
        cfg_btn_quit_game->text      = "Quitter la partie";
        cfg_btn_quit_game->callback  = game_button_click;
        btn_quit_game = button_create(context->renderer, 0, cfg_btn_quit_game);
        if (!btn_quit_game) {
            loading_fails++;
        }
        free(cfg_btn_quit_game);
    } else {
        loading_fails++;
    }

    return loading_fails;
}

/* Dessine un rectangle avec transparence */
static void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_Rect rect;
    rect.x = x + WIN_WIDTH / 2;
    rect.y = WIN_HEIGHT / 2 - y;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

/* Affiche les panneaux des équipes avec les joueurs */
void game_render_team_panels(AppContext* context) {
    const int PANEL_W = 220;
    const int PANEL_H = 400;
    const int PANEL_Y = 200;
    const int PANEL_MARGIN = 30;
    const int ICON_SPACING = 55;  /* Espacement vertical entre les joueurs */
    
    /* Panel équipe bleue (gauche) */
    int blue_x = -WIN_WIDTH/2 + PANEL_MARGIN;
    int blue_text_x = blue_x + PANEL_W/2;  /* Centre du panneau en coordonnées centrées */
    draw_filled_rect(context->renderer, blue_x, PANEL_Y, PANEL_W, PANEL_H, PANEL_BG_COLOR);
    draw_filled_rect(context->renderer, blue_x, PANEL_Y, PANEL_W, 40, TEAM_BLUE_COLOR);
    text_display(context->renderer, "EQUIPE BLEUE", FONT_LARABIE, 18, COL_WHITE, blue_text_x, PANEL_Y - 20, 0, 255);
    
    /* Espion bleu */
    text_display(context->renderer, "Espion:", FONT_LARABIE, 14, (SDL_Color){100, 150, 255, 255}, blue_text_x, PANEL_Y - 60, 0, 255);
    
    /* Agents bleus */
    text_display(context->renderer, "Agents:", FONT_LARABIE, 14, (SDL_Color){100, 150, 255, 255}, blue_text_x, PANEL_Y - 230, 0, 255);
    
    /* Afficher les joueurs bleus du lobby */
    if (context->lobby) {
        int spy_y = PANEL_Y - 100;
        int agent_y = PANEL_Y - 260;
        
        /* Afficher le joueur local s'il est dans l'équipe bleue */
        if (context->player_team == TEAM_BLUE) {
            if (context->player_role == ROLE_SPY) {
                display_image(context->renderer, player_icon_blue, blue_text_x - 40, spy_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                text_display(context->renderer, context->player_name ? context->player_name : "Moi", FONT_LARABIE, 14, COL_WHITE, blue_text_x + 20, spy_y, 0, 255);
                spy_y -= ICON_SPACING;
            } else if (context->player_role == ROLE_AGENT) {
                display_image(context->renderer, player_icon_blue, blue_text_x - 40, agent_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                text_display(context->renderer, context->player_name ? context->player_name : "Moi", FONT_LARABIE, 14, COL_WHITE, blue_text_x + 20, agent_y, 0, 255);
                agent_y -= ICON_SPACING;
            }
        }
        
        /* Afficher les autres joueurs bleus du lobby */
        for (int i = 0; i < context->lobby->nb_players && i < MAX_USERS; i++) {
            User* u = context->lobby->users[i];
            if (u && u->team == TEAM_BLUE) {
                if (u->role == ROLE_SPY) {
                    /* Icône + nom */
                    display_image(context->renderer, player_icon_blue, blue_text_x - 40, spy_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                    text_display(context->renderer, u->name ? u->name : "???", FONT_LARABIE, 14, COL_WHITE, blue_text_x + 20, spy_y, 0, 255);
                    spy_y -= ICON_SPACING;
                } else if (u->role == ROLE_AGENT) {
                    /* Icône + nom */
                    display_image(context->renderer, player_icon_blue, blue_text_x - 40, agent_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                    text_display(context->renderer, u->name ? u->name : "???", FONT_LARABIE, 14, COL_WHITE, blue_text_x + 20, agent_y, 0, 255);
                    agent_y -= ICON_SPACING;
                }
            }
        }
    }
    
    /* Panel équipe rouge (droite) */
    int red_x = WIN_WIDTH/2 - PANEL_W - PANEL_MARGIN;
    int red_text_x = red_x + PANEL_W/2;  /* Centre du panneau en coordonnées centrées */
    draw_filled_rect(context->renderer, red_x, PANEL_Y, PANEL_W, PANEL_H, PANEL_BG_COLOR);
    draw_filled_rect(context->renderer, red_x, PANEL_Y, PANEL_W, 40, TEAM_RED_COLOR);
    text_display(context->renderer, "EQUIPE ROUGE", FONT_LARABIE, 18, COL_WHITE, red_text_x, PANEL_Y - 20, 0, 255);
    
    /* Espion rouge */
    text_display(context->renderer, "Espion:", FONT_LARABIE, 14, (SDL_Color){255, 100, 100, 255}, red_text_x, PANEL_Y - 60, 0, 255);
    
    /* Agents rouges */
    text_display(context->renderer, "Agents:", FONT_LARABIE, 14, (SDL_Color){255, 100, 100, 255}, red_text_x, PANEL_Y - 230, 0, 255);
    
    /* Afficher les joueurs rouges du lobby */
    if (context->lobby) {
        int spy_y = PANEL_Y - 100;
        int agent_y = PANEL_Y - 260;
        
        /* Afficher le joueur local s'il est dans l'équipe rouge */
        if (context->player_team == TEAM_RED) {
            if (context->player_role == ROLE_SPY) {
                display_image(context->renderer, player_icon_red, red_text_x - 40, spy_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                text_display(context->renderer, context->player_name ? context->player_name : "Moi", FONT_LARABIE, 14, COL_WHITE, red_text_x + 20, spy_y, 0, 255);
                spy_y -= ICON_SPACING;
            } else if (context->player_role == ROLE_AGENT) {
                display_image(context->renderer, player_icon_red, red_text_x - 40, agent_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                text_display(context->renderer, context->player_name ? context->player_name : "Moi", FONT_LARABIE, 14, COL_WHITE, red_text_x + 20, agent_y, 0, 255);
                agent_y -= ICON_SPACING;
            }
        }
        
        /* Afficher les autres joueurs rouges du lobby */
        for (int i = 0; i < context->lobby->nb_players && i < MAX_USERS; i++) {
            User* u = context->lobby->users[i];
            if (u && u->team == TEAM_RED) {
                if (u->role == ROLE_SPY) {
                    /* Icône + nom */
                    display_image(context->renderer, player_icon_red, red_text_x - 40, spy_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                    text_display(context->renderer, u->name ? u->name : "???", FONT_LARABIE, 14, COL_WHITE, red_text_x + 20, spy_y, 0, 255);
                    spy_y -= ICON_SPACING;
                } else if (u->role == ROLE_AGENT) {
                    /* Icône + nom */
                    display_image(context->renderer, player_icon_red, red_text_x - 40, agent_y, 0.20, 0, SDL_FLIP_NONE, 1, 255);
                    text_display(context->renderer, u->name ? u->name : "???", FONT_LARABIE, 14, COL_WHITE, red_text_x + 20, agent_y, 0, 255);
                    agent_y -= ICON_SPACING;
                }
            }
        }
    }
}

void game_render_cards(AppContext * context) {
    int x=-400;
    int y=-250;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Word word = context->lobby->game->words[i*5 + j];

            if(context->player_role == ROLE_SPY && !word.revealed) {
                switch (word.team) {
                    case TEAM_NONE:
                        if (word.gender) {
                            display_image(context->renderer, card_f_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_RED:
                        if (word.gender) {
                            display_image(context->renderer, card_f_red, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }else{
                            display_image(context->renderer, card_h_red, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                        }
                        break;
                    case TEAM_BLUE:
                        if (word.gender) {
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
            }else if (context->player_role == ROLE_AGENT && !word.revealed) {
                if (word.gender) {
                    display_image(context->renderer, card_f_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                }else{
                    display_image(context->renderer, card_h_classic, x, y, 0.04, 0, SDL_FLIP_NONE, 1, 255);
                }
            }
            else if (word.revealed) {
                // Attente des images des cartes "révélées"
            }
            text_display(context->renderer, word.word, FONT_LARABIE, 18, COL_BLACK, x, y-22, 0, 255);
            x += 200;
        }
        x = -400;
        y += 125;
    }
}

void game_display(AppContext * context) {

    if (!audio_is_playing(MUSIC_GAME)) {
        audio_play(MUSIC_GAME, -1);
    }

    game_render_team_panels(context);
    game_render_cards(context);
    if (btn_quit_game) {
        button_render(context->renderer, btn_quit_game);
    }
    
}


int game_free() {
    if (card_h_classic) { free_image(card_h_classic); card_h_classic = NULL; }
    if (card_f_classic) { free_image(card_f_classic); card_f_classic = NULL; }
    if (card_h_red) { free_image(card_h_red); card_h_red = NULL; }
    if (card_f_red) { free_image(card_f_red); card_f_red = NULL; }
    if (card_h_blue) { free_image(card_h_blue); card_h_blue = NULL; }
    if (card_f_blue) { free_image(card_f_blue); card_f_blue = NULL; }
    if (card_black) { free_image(card_black); card_black = NULL; }
    if (btn_quit_game) { button_destroy(btn_quit_game); btn_quit_game = NULL; }
    return EXIT_SUCCESS;
}