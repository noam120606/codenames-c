#include "../lib/all.h"

SDL_Texture* card_h_classic;
SDL_Texture* card_f_classic;
SDL_Texture* card_h_red;
SDL_Texture* card_f_red;
SDL_Texture* card_h_blue;
SDL_Texture* card_f_blue;
SDL_Texture* card_black;

Button* btn_quit_game = NULL;

Input* hint_input = NULL;
Input* hint_count_input = NULL;
Input* chat_input = NULL;

static const char* HINT_INPUT_PLACEHOLDERS[] = {"Entrez un mot"};
static const char* HINT_COUNT_INPUT_PLACEHOLDERS[] = {"1","2","3"};
static const char* CHAT_INPUT_PLACEHOLDERS[] = {"Chattez ici ..."};

Window* blue_panel = NULL;
Window* red_panel = NULL;
Window* history_window_blue = NULL;
Window* history_window_red = NULL;
Window* hint_window = NULL;

/* Utilisation des icônes déjà chargées dans lobby.c */
extern SDL_Texture* player_icon_red;
extern SDL_Texture* player_icon_blue;

/* Couleurs pour les panneaux d'équipe */
static const SDL_Color TEAM_BLUE_COLOR = {50, 80, 150, 200};
static const SDL_Color TEAM_RED_COLOR = {150, 50, 50, 200};

/* Textes optimisés pour le jeu */
static Text* txt_team_blue_title = NULL;
static Text* txt_team_red_title = NULL;
static Text* txt_blue_spy_label = NULL;
static Text* txt_blue_agents_label = NULL;
static Text* txt_red_spy_label = NULL;
static Text* txt_red_agents_label = NULL;

static Text* txt_turn_label = NULL;

/* Textes pour les noms de joueurs (max 8 par équipe) */
#define MAX_TEAM_PLAYERS 8
static Text* txt_blue_players[MAX_TEAM_PLAYERS] = {NULL};
static Text* txt_red_players[MAX_TEAM_PLAYERS] = {NULL};
static int blue_player_text_index = 0;
static int red_player_text_index = 0;

/* Textes pour les mots des cartes (25 cartes) */
#define NUM_CARDS 25
static Text* txt_card_words[NUM_CARDS] = {NULL};

static void hint_on_submit(AppContext* context, const char* text) {
    printf("Hint input submitted: %s\n", text ? text : "");
    
    /* Envoi de l'indice au serveur */
    int valid = valid_hint(text, context->lobby->game->words);

    if (text && valid) {
        window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, &COL_WHITE); // Réinitialiser la couleur du label de soumission
        window_edit_cfg(hint_window, WIN_CFG_TITLE, "Saisissez un mot indice"); // Réinitialiser le label de soumission
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SUBMIT_HINT, text);
        send_tcp(context->sock, msg);
    } else { // TODO : Envoyer un message
        window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, valid ? &COL_GREEN : &COL_RED); // Mettre à jour la couleur du label de soumission pour indiquer que le mot est invalide
        window_edit_cfg(hint_window, WIN_CFG_TITLE, "Le mot saisi est invalide"); // Mettre à jour le label de soumission pour indiquer que le mot est invalide
        printf("Invalid hint submitted: %s\n", text ? text : "");
    }
}

static void chat_on_submit(AppContext* context, const char* text) {
    printf("Chat input submitted: %s\n", text ? text : "");
    if (text && strlen(text) > 0) {
        char msg[256];
        format_to(msg, sizeof(msg), "%d %s", MSG_SENDCHAT, text);
        send_tcp(context->sock, msg);
    }
}

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

    if (hint_input) {
        input_handle_event(context, hint_input, e);
    }
    if (hint_count_input) {
        input_handle_event(context, hint_count_input, e);
    }
    if (chat_input) {
        input_handle_event(context, chat_input, e);
    }

    if (blue_panel) {
        window_handle_event(context, blue_panel, e);
    }
    if (red_panel) {
        window_handle_event(context, red_panel, e);
    }
    if (history_window_blue) {
        window_handle_event(context, history_window_blue, e);
    }
    if (history_window_red) {
        window_handle_event(context, history_window_red, e);
    }

    if (hint_window) {
        window_handle_event(context, hint_window, e);
    }

    if (btn_quit_game) {
        button_handle_event(context, btn_quit_game, e);
    }
}

int game_init(AppContext * context) {

    int loading_fails = 0;

    // Chargement image
    card_h_classic = load_image(context->renderer, "assets/img/cards/none/H.png");
    if (!card_h_classic) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_classic = load_image(context->renderer, "assets/img/cards/none/F.png");
    if (!card_f_classic) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_h_red = load_image(context->renderer, "assets/img/cards/red/H.png");
    if (!card_h_red) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_red = load_image(context->renderer, "assets/img/cards/red/F.png");
    if (!card_f_red) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_h_blue = load_image(context->renderer, "assets/img/cards/blue/H.png");
    if (!card_h_blue) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_f_blue = load_image(context->renderer, "assets/img/cards/blue/F.png");
    if (!card_f_blue) {
        printf("Failed to load card image\n");
        loading_fails++;
    }
    card_black = load_image(context->renderer, "assets/img/cards/black/B.png");
    if (!card_black) {
        printf("Failed to load card image\n");
        loading_fails++;
    }

    ButtonConfig* cfg_btn_quit_game = button_config_init();
    if (cfg_btn_quit_game) {
        cfg_btn_quit_game->x         = -775;
        cfg_btn_quit_game->y         = 450;
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

    InputConfig* cfg_hint_input = input_config_init();
    if (cfg_hint_input) {
        cfg_hint_input->x = 0;
        cfg_hint_input->y = -450;
        cfg_hint_input->w = 500;
        cfg_hint_input->h = 64;
        cfg_hint_input->font_path = FONT_LARABIE;
        cfg_hint_input->font_size = 24;
        cfg_hint_input->placeholders = HINT_INPUT_PLACEHOLDERS;
        cfg_hint_input->placeholder_count = 1;
        cfg_hint_input->maxlen = 50;
        cfg_hint_input->centered = 1;
        cfg_hint_input->on_submit = hint_on_submit;
        cfg_hint_input->allowed_pattern = "^[A-Za-z]$";
        cfg_hint_input->submit_pattern = "^[A-Za-z]{50}$";
        cfg_hint_input->bg_path = "assets/img/inputs/empty.png";
        cfg_hint_input->bg_padding = 16;
        hint_input = input_create(context->renderer, INPUT_HINT, cfg_hint_input);
        if (!hint_input) {
            loading_fails++;
        }
        free(cfg_hint_input);
    } else {
        loading_fails++;
    }

    InputConfig* cfg_hint_count_input = input_config_init();
    if (cfg_hint_count_input) {
        cfg_hint_count_input->x = 300;
        cfg_hint_count_input->y = -450;
        cfg_hint_count_input->w = 70;
        cfg_hint_count_input->h = 64;
        cfg_hint_count_input->font_path = FONT_LARABIE;
        cfg_hint_count_input->font_size = 24;
        cfg_hint_count_input->placeholders = HINT_COUNT_INPUT_PLACEHOLDERS;
        cfg_hint_count_input->placeholder_count = 3;
        cfg_hint_count_input->maxlen = 1;
        cfg_hint_count_input->centered = 1;
        cfg_hint_count_input->allowed_pattern = "^[0-9]$";
        cfg_hint_count_input->submit_pattern = "^[0-9]{1}$";
        cfg_hint_count_input->bg_path = "assets/img/inputs/empty.png";
        cfg_hint_count_input->bg_padding = 10;
        hint_count_input = input_create(context->renderer, INPUT_HINT_COUNT, cfg_hint_count_input);
        if (!hint_count_input) {
            loading_fails++;
        }
        free(cfg_hint_count_input);
    } else {
        loading_fails++;
    }

    InputConfig* cfg_chat_input = input_config_init();
    if (cfg_chat_input) {
        cfg_chat_input->x = 750;
        cfg_chat_input->y = 450;
        cfg_chat_input->w = 260;
        cfg_chat_input->h = 64;
        cfg_chat_input->font_path = FONT_LARABIE;
        cfg_chat_input->font_size = 24;
        cfg_chat_input->placeholders = CHAT_INPUT_PLACEHOLDERS;
        cfg_chat_input->placeholder_count = 1;
        cfg_chat_input->maxlen = 50;
        cfg_chat_input->centered = 1;
        cfg_chat_input->allowed_pattern = NULL;
        cfg_chat_input->submit_pattern = NULL;
        cfg_chat_input->bg_path = "assets/img/inputs/empty.png";
        cfg_chat_input->bg_padding = 16;
        chat_input = input_create(context->renderer, INPUT_CHAT, cfg_chat_input);
        if (!chat_input) {
            loading_fails++;
        }
        free(cfg_chat_input);
    } else {
        loading_fails++;
    }

    /* Initialisation des textes optimisés pour les panneaux */
    txt_team_blue_title = init_text(context, "EQUIPE BLEUE", 
        create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 0, 0, 255));
    txt_team_red_title = init_text(context, "EQUIPE ROUGE", 
        create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 0, 0, 255));
    
    txt_blue_spy_label = init_text(context, "Espion:", 
        create_text_config(FONT_LARABIE, 14, (SDL_Color){100, 150, 255, 255}, 0, 0, 0, 255));
    txt_blue_agents_label = init_text(context, "Agents:", 
        create_text_config(FONT_LARABIE, 14, (SDL_Color){100, 150, 255, 255}, 0, 0, 0, 255));
    txt_red_spy_label = init_text(context, "Espion:", 
        create_text_config(FONT_LARABIE, 14, (SDL_Color){255, 100, 100, 255}, 0, 0, 0, 255));
    txt_red_agents_label = init_text(context, "Agents:", 
        create_text_config(FONT_LARABIE, 14, (SDL_Color){255, 100, 100, 255}, 0, 0, 0, 255));

    txt_turn_label = init_text(context, "", 
        create_text_config(FONT_LARABIE, 32, COL_WHITE, 0, 0, 0, 255));

    /* Textes pour les noms de joueurs */
    for (int i = 0; i < MAX_TEAM_PLAYERS; i++) {
        txt_blue_players[i] = init_text(context, " ", 
            create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 0, 0, 255));
        txt_red_players[i] = init_text(context, " ", 
            create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 0, 0, 255));
    }

    /* Textes pour les mots des cartes */
    for (int i = 0; i < NUM_CARDS; i++) {
        txt_card_words[i] = init_text(context, " ", 
            create_text_config(FONT_LARABIE, 18, COL_BLACK, 0, 0, 0, 255));
    }

    /* Chargement de la fenêtre de l'équipe bleue */
    WindowConfig* cfg_blue_panel = window_config_init();
    if (cfg_blue_panel) {
        cfg_blue_panel->x = 775;
        cfg_blue_panel->y = 150;
        cfg_blue_panel->w = 250;
        cfg_blue_panel->h = 350;
        cfg_blue_panel->title = "Equipe bleue";
        cfg_blue_panel->titlebar_color = TEAM_BLUE_COLOR;
        cfg_blue_panel->bg_color = (SDL_Color){20, 20, 20, 220};
        blue_panel = window_create(0, cfg_blue_panel);
        if (!blue_panel) {
            loading_fails++;
        }
        free(cfg_blue_panel);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre de l'équipe rouge */
    WindowConfig* cfg_red_panel = window_config_init();
    if (cfg_red_panel) {
        cfg_red_panel->x = -775;
        cfg_red_panel->y = 150;
        cfg_red_panel->w = 250;
        cfg_red_panel->h = 350;
        cfg_red_panel->title = "Equipe rouge";
        cfg_red_panel->bg_color = (SDL_Color){20, 20, 20, 220};
        cfg_red_panel->titlebar_color = TEAM_RED_COLOR;
        red_panel = window_create(1, cfg_red_panel);
        if (!red_panel) {
            loading_fails++;
        }
        free(cfg_red_panel);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre de l'historique del'équipe bleue */
    WindowConfig* cfg_history_blue = window_config_init();
    if (cfg_history_blue) {
        cfg_history_blue->x = 775;
        cfg_history_blue->y = -350;
        cfg_history_blue->w = 300;
        cfg_history_blue->h = 300;
        cfg_history_blue->title = "Historique bleu";
        cfg_history_blue->titlebar_color = TEAM_BLUE_COLOR;
        cfg_history_blue->bg_color = (SDL_Color){20, 20, 20, 220};
        history_window_blue = window_create(0, cfg_history_blue);
        if (!history_window_blue) {
            loading_fails++;
        }
        free(cfg_history_blue);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre de l'historique de l'équipe rouge */
    WindowConfig* cfg_history_red = window_config_init();
    if (cfg_history_red) {
        cfg_history_red->x = -775;
        cfg_history_red->y = -350;
        cfg_history_red->w = 300;
        cfg_history_red->h = 300;
        cfg_history_red->title = "Historique rouge";
        cfg_history_red->titlebar_color = TEAM_RED_COLOR;
        cfg_history_red->bg_color = (SDL_Color){20, 20, 20, 220};
        history_window_red = window_create(1, cfg_history_red);
        if (!history_window_red) {
            loading_fails++;
        }
        free(cfg_history_red);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre de saisie des mots */
    WindowConfig* cfg_hint_window = window_config_init();
    if (cfg_hint_window) {
        cfg_hint_window->x = 0;
        cfg_hint_window->y = -400;
        cfg_hint_window->w = 650;
        cfg_hint_window->h = 120;
        cfg_hint_window->title = "";
        cfg_hint_window->movable = 1;
        cfg_hint_window->titlebar_color = COL_GRAY;
        cfg_hint_window->bg_color = (SDL_Color){20, 20, 20, 220};
        hint_window = window_create(1, cfg_hint_window);
        if (!hint_window) {
            loading_fails++;
        }
        free(cfg_hint_window);
    } else {
        loading_fails++;
    }

    return loading_fails;
}

static void render_team_member_in_panel(AppContext* context, Window* panel, SDL_Texture* icon, Text* txt, const char* name, UserRole role, int* spy_row, int* agent_row) {
    if (!context || !panel || !icon || !txt || !spy_row || !agent_row) return;

    const int POS_X = 0;
    const int SPY_BASE_Y = 50;
    const int AGENT_BASE_Y = 125 - 25;
    const int ROW_GAP = 36;

    int row = (role == ROLE_SPY) ? *spy_row : *agent_row;
    int icon_y = ((role == ROLE_SPY) ? SPY_BASE_Y : AGENT_BASE_Y) + (row * ROW_GAP);

    window_display_image(context->renderer, panel, icon, POS_X, icon_y+20, 0.20f, 0, SDL_FLIP_NONE, 1, 255);

    update_text(context, txt, name ? name : "???");
    window_place_text(panel, txt, POS_X, icon_y);
    display_text(context, txt);

    if (role == ROLE_SPY) (*spy_row)++;
    else (*agent_row)++;
}

static void render_team_panel_content(AppContext* context, Window* panel, Team team, SDL_Texture* icon, Text* txt_spy_label, Text* txt_agents_label, Text** player_texts, int* player_index) {
    if (!context || !panel || !txt_spy_label || !txt_agents_label || !player_texts || !player_index) return;

    int spy_row = 0;
    int agent_row = 0;

    update_text(context, txt_spy_label, "Espions :");
    window_place_text(panel, txt_spy_label, 0, -25);
    display_text(context, txt_spy_label);

    update_text(context, txt_agents_label, "Agents :");
    window_place_text(panel, txt_agents_label, 0, 125);
    display_text(context, txt_agents_label);

    *player_index = 0;

    if (!context->lobby) return;

    if (context->player_team == team && *player_index < MAX_TEAM_PLAYERS) {
        Text* txt = player_texts[*player_index];
        const char* local_name = context->player_name ? context->player_name : "Moi";
        render_team_member_in_panel(context, panel, icon, txt, local_name, context->player_role, &spy_row, &agent_row);
        (*player_index)++;
    }

    for (int i = 0; i < context->lobby->nb_players && i < MAX_USERS; i++) {
        User* u = context->lobby->users[i];
        if (!u || u->team != team || *player_index >= MAX_TEAM_PLAYERS) continue;

        Text* txt = player_texts[*player_index];
        render_team_member_in_panel(context, panel, icon, txt, u->name ? u->name : "???", u->role, &spy_row, &agent_row);
        (*player_index)++;
    }
}

static void game_render_team_windows(AppContext* context) {
    if (!context) return;

    if (blue_panel) {
        render_team_panel_content(context, blue_panel, TEAM_BLUE, player_icon_blue, txt_blue_spy_label, txt_blue_agents_label, txt_blue_players, &blue_player_text_index);
    }
    if (red_panel) {
        render_team_panel_content(context, red_panel, TEAM_RED, player_icon_red, txt_red_spy_label, txt_red_agents_label, txt_red_players, &red_player_text_index);
    }
}

void game_render_cards(AppContext * context) {
    int x=-400;
    int y=-250;
    int card_index = 0;
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
            
            /* Affichage du mot de la carte avec le nouveau système */
            if (card_index < NUM_CARDS && txt_card_words[card_index]) {
                update_text(context, txt_card_words[card_index], word.word);
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

void game_display(AppContext * context) {

    if (!audio_is_playing(MUSIC_GAME)) {
        audio_play(MUSIC_GAME, -1);
    }

    game_render_cards(context);
    if (btn_quit_game) {
        button_render(context->renderer, btn_quit_game);
    }
    

    if (chat_input) {
        input_render(context->renderer, chat_input);
    }

    if (blue_panel) {
        window_render(context->renderer, blue_panel);
    }
    if (red_panel) {
        window_render(context->renderer, red_panel);
    }
    if (history_window_blue) {
        window_render(context->renderer, history_window_blue);
    }
    if (history_window_red) {
        window_render(context->renderer, history_window_red);
    }

    if (hint_window) {
        window_render(context->renderer, hint_window);

        const char* turn_text = "";
        SDL_Color color_text;

        switch (context->lobby->game->state) {
            case GAMESTATE_TURN_BLUE_SPY:
                turn_text = "Tour de l'espion bleu";
                color_text = TEAM_BLUE_COLOR;
                break;
            case GAMESTATE_TURN_BLUE_AGENT:
                turn_text = "Tour de l'agent bleu";
                color_text = TEAM_BLUE_COLOR;
                break;
            case GAMESTATE_TURN_RED_SPY:
                turn_text = "Tour de l'espion rouge";
                color_text = TEAM_RED_COLOR;
                break;
            case GAMESTATE_TURN_RED_AGENT:
                turn_text = "Tour de l'agent rouge";
                color_text = TEAM_RED_COLOR;
                break;
            default:
                turn_text = "Phase de préparation ou partie terminée";
                break;
        }

        if (context->player_role == ROLE_SPY &&
            (context->lobby->game->state == GAMESTATE_TURN_RED_SPY && context->player_team == TEAM_RED) ||
            (context->lobby->game->state == GAMESTATE_TURN_BLUE_SPY && context->player_team == TEAM_BLUE)
        ) {
            if (hint_input) {
                window_place_input(hint_window, hint_input, -40, -15);
                input_render(context->renderer, hint_input);
            }
            if (hint_count_input) {
                window_place_input(hint_window, hint_count_input, 260, -15);
                input_render(context->renderer, hint_count_input);
            }
        } else {
            update_text(context, txt_turn_label, turn_text);
            update_text_color(context, txt_turn_label, color_text);
            window_place_text(hint_window, txt_turn_label, 0, -16);
            display_text(context, txt_turn_label);
        }

    }

    game_render_team_windows(context);
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
    if (hint_input) { input_destroy(hint_input); hint_input = NULL; }
    if (hint_count_input) { input_destroy(hint_count_input); hint_count_input = NULL; }
    if (chat_input) { input_destroy(chat_input); chat_input = NULL; }
    if (blue_panel) { window_destroy(blue_panel); blue_panel = NULL; }
    if (red_panel) { window_destroy(red_panel); red_panel = NULL; }
    if (history_window_blue) { window_destroy(history_window_blue); history_window_blue = NULL; }
    if (history_window_red) { window_destroy(history_window_red); history_window_red = NULL; }
    if (hint_window) { window_destroy(hint_window); hint_window = NULL; }

    /* Libération des textes optimisés */
    destroy_text(txt_team_blue_title); txt_team_blue_title = NULL;
    destroy_text(txt_team_red_title); txt_team_red_title = NULL;
    destroy_text(txt_blue_spy_label); txt_blue_spy_label = NULL;
    destroy_text(txt_blue_agents_label); txt_blue_agents_label = NULL;
    destroy_text(txt_red_spy_label); txt_red_spy_label = NULL;
    destroy_text(txt_red_agents_label); txt_red_agents_label = NULL;
    destroy_text(txt_turn_label); txt_turn_label = NULL;

    for (int i = 0; i < MAX_TEAM_PLAYERS; i++) {
        destroy_text(txt_blue_players[i]); txt_blue_players[i] = NULL;
        destroy_text(txt_red_players[i]); txt_red_players[i] = NULL;
    }
    
    for (int i = 0; i < NUM_CARDS; i++) {
        destroy_text(txt_card_words[i]); txt_card_words[i] = NULL;
    }
    
    return EXIT_SUCCESS;
}
