#include "../lib/all.h"

Button* btn_quit_game = NULL;
Button* btn_hint_submit = NULL;
Button* btn_return_lobby = NULL;

Input* hint_input = NULL;
Input* hint_count_input = NULL;
Input* chat_input = NULL;

static const char* HINT_INPUT_PLACEHOLDERS[] = {"Entrez un mot indice"};
static const char* HINT_COUNT_INPUT_PLACEHOLDERS[] = {"1","2","3"};
static const char* CHAT_INPUT_PLACEHOLDERS[] = {"Chattez ici ..."};

Window* blue_panel = NULL;
Window* red_panel = NULL;
Window* history_window_blue = NULL;
Window* history_window_red = NULL;
Window* hint_window = NULL;
Window* chat_window = NULL;

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
static Text* txt_hint_display = NULL;

/* Textes pour les noms de joueurs (max 8 par équipe) */
#define MAX_TEAM_PLAYERS 8
static Text* txt_blue_players[MAX_TEAM_PLAYERS] = {NULL};
static Text* txt_red_players[MAX_TEAM_PLAYERS] = {NULL};
static int blue_player_text_index = 0;
static int red_player_text_index = 0;

/* Textes pour l'affichage des derniers messages du chat */
#define CHAT_VISIBLE_LINES 10
static Text* txt_chat_messages[CHAT_VISIBLE_LINES] = {NULL};

/* Textes pour l'affichage de l'historique des tours */
#define HISTORY_VISIBLE_LINES 14
#define HISTORY_MAX_LINES ((NB_WORDS * 2) + 4) // Max 2 tours par équipe + 4 lignes de séparation et d'information
static Text* txt_history_blue_lines[HISTORY_VISIBLE_LINES] = {NULL};
static Text* txt_history_red_lines[HISTORY_VISIBLE_LINES] = {NULL};

static void hint_on_submit(AppContext* context, const char* text) {
    (void)context;
    (void)text;
    if (hint_count_input) {
        hint_count_input->cfg->focused = 1;
        hint_count_input->cfg->cursor_pos = strlen(hint_count_input->cfg->text);
    }
    // Attendre que le joueur ne presse plus la touche Entrée ou Entrée du numpad
    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYUP && (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)) {
                break;
            }
        }
    }
}

static void hint_count_on_submit(AppContext* context, const char* text) {
    (void)text;

    const char* hint_text = (hint_input && hint_input->cfg) ? hint_input->cfg->text : NULL;

    // On vérifie que le champ de saisie du mot indice n'est pas vide avant d'exécuter l'action du bouton de soumission de l'indice.
    if (hint_text && strlen(hint_text) > 0) {
        if (btn_hint_submit && btn_hint_submit->cfg && btn_hint_submit->cfg->callback) {
            btn_hint_submit->cfg->callback(context, btn_hint_submit);
        }
    } else if (hint_input && hint_input->cfg) {
        // Si le champ de saisie du mot indice est vide, on focus sur le champ de saisie du mot indice pour inviter l'utilisateur à saisir un mot indice.
        hint_input->cfg->focused = 1;
        hint_input->cfg->cursor_pos = hint_input->cfg->text ? strlen(hint_input->cfg->text) : 0;
    }
}

// Réinitialiser le champ de saisie du mot indice et du compteur d'indices après l'envoi du message
static void clear_hint_inputs(){
    if (hint_input && hint_input->cfg) {
        input_set_text(hint_input, "");
    }
    if (hint_count_input && hint_count_input->cfg) {
        input_set_text(hint_count_input, "");
    }
}

static ButtonReturn game_button_click(AppContext* context, Button* button) {
    if (!context || !button) return BTN_RET_NONE;

    if (button == btn_quit_game) {
        /* Retirer le filtre audio en quittant game */
        audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_NONE, 0);
        context->app_state = APP_STATE_MENU;
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;

        struct_lobby_init(context->lobby, -1, "");
        char msg[16];
        format_to(msg, sizeof(msg), "%d", MSG_LEAVELOBBY);
        send_tcp(context->sock, msg);

        printf("Left game and returned to menu\n");
        clear_hint_inputs();

        return BTN_RET_QUIT;
    }

    if (button == btn_hint_submit) {

        if (context->player_role == ROLE_AGENT) {
            char msg[64];
            format_to(msg, sizeof(msg), "%d -1", MSG_GUESS_CARD);
            send_tcp(context->sock, msg);
            return BTN_RET_NONE;
        }

        char* text = hint_input->cfg->text;
        int nb_hint = (int)atoi(hint_count_input->cfg->text);
        int valid = valid_hint(text, context->lobby->game->cards);
        
        
        char title[64];
        /* Envoi de l'indice au serveur */
        // On stocke cette chaine : "Vous avez soumis le mot : " et on lui ajoute "text" et "en nb_hint"
        if (text && nb_hint && valid) {
            format_to(title, sizeof(title), "Vous avez soumis le mot : %s en %d", text ? text : "", nb_hint);
            window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&COL_DARK_GREEN); // Mettre à jour la couleur du label de soumission pour indiquer que le mot est valide
            window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)title); // Mettre à jour le label de soumission
            char msg[64];
            format_to(msg, sizeof(msg), "%d %s %d %s", MSG_SUBMIT_HINT, context->player_name ? context->player_name : "Unknown", nb_hint, text);
            send_tcp(context->sock, msg);
            clear_hint_inputs();
        } else if(text == NULL || strlen(text) == 0) {
            window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&COL_RED); // Mettre à jour la couleur du label de soumission pour indiquer qu'aucun mot n'a été saisi
            window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)"Vous n'avez pas saisi de mot"); // Mettre à jour le label de soumission pour indiquer qu'aucun mot n'a été saisi
            printf("Invalid hint submitted: %s\n", text ? text : "");
        } else if(nb_hint <= 0) {
            window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&COL_RED); // Mettre à jour la couleur du label de soumission pour indiquer que le nombre d'indices est invalide
            window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)"Vous n'avez pas saisi de nombre d'indices"); // Mettre à jour le label de soumission pour indiquer que le nombre d'indices est invalide
            printf("No hint count submitted\n");
        } else {
            format_to(title, sizeof(title), "Le mot : %s est invalide", text ? text : "");
            window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&COL_RED); // Mettre à jour la couleur du label de soumission pour indiquer que le mot est invalide
            window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)title); // Mettre à jour le label de soumission pour indiquer que le mot est invalide
            printf("Invalid hint submitted: %s\n", text ? text : "");
        }

        return BTN_RET_NONE;
    }

    if (button == btn_return_lobby) {
        context->app_state = APP_STATE_LOBBY;
        printf("Returned to lobby\n");
        game_struct_free(context);
        return BTN_RET_NONE;
    }

    return BTN_RET_NONE;
}

int my_turn(AppContext* context) {
    switch (context->player_role) {
        case ROLE_SPY: return context->lobby->game->state == (context->player_team == TEAM_RED ? GAMESTATE_TURN_RED_SPY : GAMESTATE_TURN_BLUE_SPY);
        case ROLE_AGENT: return context->lobby->game->state == (context->player_team == TEAM_RED ? GAMESTATE_TURN_RED_AGENT : GAMESTATE_TURN_BLUE_AGENT);
        default: return 0;
    }
}

int game_struct_free(AppContext* context) {
    if (!context || !context->lobby || !context->lobby->game) return EXIT_FAILURE;
    free(context->lobby->game->cards);
    free(context->lobby->game);
    context->lobby->game = NULL;
    return EXIT_SUCCESS;
}

void game_handle_event(AppContext* context, SDL_Event* e) {
    if (!context || !e) return;

    // Déterminer les éléments à gérer en fonction de l'état du jeu et du rôle du joueur
    int has_game = (context->lobby && context->lobby->game);
    int is_my_turn = has_game ? my_turn(context) : 0;
    int show_spy_hint_controls = (context->player_role == ROLE_SPY && is_my_turn);
    int has_hint = has_game && (context->lobby->game->current_hint[0] != '\0' && context->lobby->game->current_hint_count > 0);
    int show_hint_for_agents_or_opponents = has_game && has_hint && (
        (context->lobby->game->state == GAMESTATE_TURN_BLUE_AGENT && (context->player_team == TEAM_RED || context->player_role == ROLE_AGENT)) ||
        (context->lobby->game->state == GAMESTATE_TURN_RED_AGENT && (context->player_team == TEAM_BLUE || context->player_role == ROLE_AGENT))
    );
    int show_agent_hint_button = (show_hint_for_agents_or_opponents && context->player_role == ROLE_AGENT && is_my_turn);
    int show_return_lobby_button = (has_game && context->lobby->game->state == GAMESTATE_ENDED);

    if (btn_quit_game) button_handle_event(context, btn_quit_game, e);
    if (btn_hint_submit && (show_spy_hint_controls || show_agent_hint_button)) button_handle_event(context, btn_hint_submit, e);
    if (btn_return_lobby && show_return_lobby_button) button_handle_event(context, btn_return_lobby, e);

    if (hint_input && show_spy_hint_controls) input_handle_event(context, hint_input, e);
    if (hint_count_input && show_spy_hint_controls) input_handle_event(context, hint_count_input, e);
    if (chat_input) input_handle_event(context, chat_input, e);

    if (blue_panel) window_handle_event(context, blue_panel, e);
    if (red_panel) window_handle_event(context, red_panel, e);
    if (history_window_blue) window_handle_event(context, history_window_blue, e);
    if (history_window_red) window_handle_event(context, history_window_red, e);

    if (hint_window) window_handle_event(context, hint_window, e);

    if (chat_window) window_handle_event(context, chat_window, e);
}

int game_init(AppContext * context) {

    int loading_fails = 0;

    ButtonConfig* cfg_btn_quit_game = button_config_init();
    if (cfg_btn_quit_game) {
        cfg_btn_quit_game->x = -775;
        cfg_btn_quit_game->y = 450;
        cfg_btn_quit_game->h = 64;
        cfg_btn_quit_game->font_path = FONT_LARABIE;
        cfg_btn_quit_game->color = COL_WHITE;
        cfg_btn_quit_game->text = "Quitter la partie";
        cfg_btn_quit_game->callback = game_button_click;
        btn_quit_game = button_create(context->renderer, 0, cfg_btn_quit_game);
        if (!btn_quit_game) loading_fails++;
        free(cfg_btn_quit_game);
    } else {
        loading_fails++;
    }

    ButtonConfig* cfg_btn_hint_submit = button_config_init();
    if (cfg_btn_hint_submit) {
        cfg_btn_hint_submit->x = 300;
        cfg_btn_hint_submit->y = -450;
        cfg_btn_hint_submit->w = 64;
        cfg_btn_hint_submit->h = 64;
        cfg_btn_hint_submit->font_path = FONT_LARABIE;
        cfg_btn_hint_submit->color = COL_WHITE;
        cfg_btn_hint_submit->text = NULL;
        cfg_btn_hint_submit->is_text = 0;
        cfg_btn_hint_submit->tex_path = "assets/img/buttons/validate1.png";
        cfg_btn_hint_submit->callback = game_button_click;
        btn_hint_submit = button_create(context->renderer, 0, cfg_btn_hint_submit);
        if (!btn_hint_submit) loading_fails++;
        free(cfg_btn_hint_submit);
    } else {
        loading_fails++;
    }

    ButtonConfig* cfg_btn_return_lobby = button_config_init();
    if (cfg_btn_return_lobby) {
        cfg_btn_return_lobby->x = 0;
        cfg_btn_return_lobby->y = 0;
        cfg_btn_return_lobby->w = 200;
        cfg_btn_return_lobby->h = 64;
        cfg_btn_return_lobby->font_path = FONT_LARABIE;
        cfg_btn_return_lobby->color = COL_WHITE;
        cfg_btn_return_lobby->text = "Retour au lobby";
        cfg_btn_return_lobby->callback = game_button_click;
        btn_return_lobby = button_create(context->renderer, 0, cfg_btn_return_lobby);
        if (!btn_return_lobby) loading_fails++;
        free(cfg_btn_return_lobby);
    } else {
        loading_fails++;
    }

    InputConfig* cfg_hint_input = input_config_init();
    if (cfg_hint_input) {
        cfg_hint_input->x = -30;
        cfg_hint_input->y = -450;
        cfg_hint_input->w = 500;
        cfg_hint_input->h = 64;
        cfg_hint_input->font_path = FONT_LARABIE;
        cfg_hint_input->font_size = 24;
        cfg_hint_input->placeholders = HINT_INPUT_PLACEHOLDERS;
        cfg_hint_input->placeholder_count = 1;
        cfg_hint_input->maxlen = 50;
        cfg_hint_input->clear_on_submit = 0;
        cfg_hint_input->centered = 1;
        cfg_hint_input->allowed_pattern = "^[A-Za-zéèêëàâäåæçîïìùûüÿœ]$";
        cfg_hint_input->submit_pattern = "^[A-Za-zéèêëàâäåæçîïìùûüÿœ]{1,50}$";
        cfg_hint_input->bg_path = "assets/img/inputs/empty.png";
        cfg_hint_input->bg_padding = 16;
        cfg_hint_input->on_submit = hint_on_submit;
        hint_input = input_create(context->renderer, INPUT_HINT, cfg_hint_input);
        if (!hint_input) loading_fails++;
        free(cfg_hint_input);
    } else {
        loading_fails++;
    }

    InputConfig* cfg_hint_count_input = input_config_init();
    if (cfg_hint_count_input) {
        cfg_hint_count_input->x = 270;
        cfg_hint_count_input->y = -450;
        cfg_hint_count_input->w = 64;
        cfg_hint_count_input->h = 64;
        cfg_hint_count_input->font_path = FONT_LARABIE;
        cfg_hint_count_input->font_size = 24;
        cfg_hint_count_input->placeholders = HINT_COUNT_INPUT_PLACEHOLDERS;
        cfg_hint_count_input->placeholder_count = 3;
        cfg_hint_count_input->maxlen = 1;
        cfg_hint_count_input->clear_on_submit = 0;
        cfg_hint_count_input->centered = 1;
        cfg_hint_count_input->allowed_pattern = "^[1-9]$";
        cfg_hint_count_input->submit_pattern = "^[1-9]{1}$";
        cfg_hint_count_input->bg_path = "assets/img/inputs/square.png";
        cfg_hint_count_input->bg_padding = 10;
        cfg_hint_count_input->on_submit = hint_count_on_submit;
        hint_count_input = input_create(context->renderer, INPUT_HINT_COUNT, cfg_hint_count_input);
        if (!hint_count_input) loading_fails++;
        free(cfg_hint_count_input);
    } else {
        loading_fails++;
    }

    InputConfig* cfg_chat_input = input_config_init();
    if (cfg_chat_input) {
        cfg_chat_input->x = 750;
        cfg_chat_input->y = 455;
        cfg_chat_input->w = 338; // Légerement plus petit que la largeur de la fenêtre pour laisser un peu de marge
        cfg_chat_input->h = 48;
        cfg_chat_input->font_path = FONT_LARABIE;
        cfg_chat_input->font_size = 24;
        cfg_chat_input->placeholders = CHAT_INPUT_PLACEHOLDERS;
        cfg_chat_input->placeholder_count = 1;
        cfg_chat_input->maxlen = 512;
        cfg_chat_input->centered = 1;
        cfg_chat_input->allowed_pattern = NULL;
        cfg_chat_input->submit_pattern = NULL;
        cfg_chat_input->submit_sound = "assets/audio/sfx/input/submit.ogg";
        cfg_chat_input->keep_focus_on_submit = 1;
        cfg_chat_input->submit_pattern = "^.{1,512}$"; // Accepter tout texte de 1 à 512 caractères
        cfg_chat_input->bg_path = "assets/img/inputs/empty.png";
        cfg_chat_input->bg_padding = 16;
        cfg_chat_input->on_submit = chat_submit_message;
        chat_input = input_create(context->renderer, INPUT_CHAT, cfg_chat_input);
        if (!chat_input) loading_fails++;
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

    txt_hint_display = init_text(context, "", 
        create_text_config(FONT_LARABIE, 28, COL_WHITE, 0, 0, 0, 255));

    /* Textes pour les noms de joueurs */
    for (int i = 0; i < MAX_TEAM_PLAYERS; i++) {
        txt_blue_players[i] = init_text(context, " ", 
            create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 0, 0, 255));
        txt_red_players[i] = init_text(context, " ", 
            create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 0, 0, 255));
    }

    /* Textes pour les messages du chat */
    for (int i = 0; i < CHAT_VISIBLE_LINES; i++) {
        txt_chat_messages[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 14, COL_WHITE, 0, 0, 0, 255));
    }

    /* Textes pour les lignes d'historique des équipes */
    for (int i = 0; i < HISTORY_VISIBLE_LINES; i++) {
        txt_history_blue_lines[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 18, COL_WHITE, 0, 0, 0, 255));
        if (!txt_history_blue_lines[i]) loading_fails++;

        txt_history_red_lines[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 18, COL_WHITE, 0, 0, 0, 255));
        if (!txt_history_red_lines[i]) loading_fails++;
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
        if (!blue_panel) loading_fails++;
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
        if (!red_panel) loading_fails++;
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
        cfg_history_blue->scrollable = 1;
        cfg_history_blue->scroll_x1 = -(cfg_history_blue->w / 2) + 4;
        cfg_history_blue->scroll_y1 = (cfg_history_blue->h / 2) - cfg_history_blue->titlebar_h - 4;
        cfg_history_blue->scroll_x2 = (cfg_history_blue->w / 2) - 4;
        cfg_history_blue->scroll_y2 = -(cfg_history_blue->h / 2) + 4;
        cfg_history_blue->scroll_step = 1;
        cfg_history_blue->scroll_min = 0;
        cfg_history_blue->scroll_max = 0;
        cfg_history_blue->scroll_offset = 0;
        history_window_blue = window_create(0, cfg_history_blue);
        if (!history_window_blue) loading_fails++;
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
        cfg_history_red->scrollable = 1;
        cfg_history_red->scroll_x1 = -(cfg_history_red->w / 2) + 4;
        cfg_history_red->scroll_y1 = (cfg_history_red->h / 2) - cfg_history_red->titlebar_h - 4;
        cfg_history_red->scroll_x2 = (cfg_history_red->w / 2) - 4;
        cfg_history_red->scroll_y2 = -(cfg_history_red->h / 2) + 4;
        cfg_history_red->scroll_step = 1;
        cfg_history_red->scroll_min = 0;
        cfg_history_red->scroll_max = 0;
        cfg_history_red->scroll_offset = 0;
        history_window_red = window_create(1, cfg_history_red);
        if (!history_window_red) loading_fails++;
        free(cfg_history_red);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre de saisie des mots */
    WindowConfig* cfg_hint_window = window_config_init();
    if (cfg_hint_window) {
        cfg_hint_window->x = 0;
        cfg_hint_window->y = -400;
        cfg_hint_window->w = 675;
        cfg_hint_window->h = 120;
        cfg_hint_window->title = "";
        cfg_hint_window->titlebar_color = COL_GRAY;
        cfg_hint_window->bg_color = (SDL_Color){20, 20, 20, 220};
        hint_window = window_create(1, cfg_hint_window);
        if (!hint_window) loading_fails++;
        free(cfg_hint_window);
    } else {
        loading_fails++;
    }

    /* Chargement de la fenêtre du chat */
    WindowConfig* cfg_chat_window = window_config_init();
    if (cfg_chat_window) {
        cfg_chat_window->x = 790;
        cfg_chat_window->y = 440;
        cfg_chat_window->w = 340;
        cfg_chat_window->h = 200;
        cfg_chat_window->title = "";
        cfg_chat_window->movable = 1;
        cfg_chat_window->titlebar_h = 0;
        cfg_chat_window->bg_color = (SDL_Color){20, 20, 20, 240};
        cfg_chat_window->scrollable = 1;
        cfg_chat_window->scroll_x1 = -(cfg_chat_window->w / 2) + 4;
        cfg_chat_window->scroll_y1 = (cfg_chat_window->h / 2) - 4;
        cfg_chat_window->scroll_x2 = (cfg_chat_window->w / 2) - 4;
        cfg_chat_window->scroll_y2 = -(cfg_chat_window->h / 2) + 4;
        cfg_chat_window->scroll_step = 1;
        cfg_chat_window->scroll_min = 0;
        cfg_chat_window->scroll_max = 0;
        cfg_chat_window->scroll_offset = 0;
        chat_window = window_create(1, cfg_chat_window);
        if (!chat_window) loading_fails++;
        free(cfg_chat_window);
    } else {
        loading_fails++;
    }

    return loading_fails;
}

typedef struct TeamPanelMember {
    Text* txt;
    const char* name;
} TeamPanelMember;

static int compute_panel_member_position(int nb_player, int i_player, int base_x, int base_y, int* out_x, int* out_y) {
    if (!out_x || !out_y) return 0;
    if (nb_player < 1 || nb_player > 8) return 0;
    if (i_player < 0 || i_player >= nb_player) return 0;

    const int spacing_x = 60;
    const int spacing_y = 64;

    int first_line_count = 0;
    switch (nb_player) {
        case 1: first_line_count = 1; break;
        case 2: first_line_count = 2; break;
        case 3: first_line_count = 3; break;
        case 4: first_line_count = 4; break;
        case 5: first_line_count = 3; break;
        case 6: first_line_count = 3; break;
        case 7: first_line_count = 4; break;
        case 8: first_line_count = 4; break;
        default: return 0;
    }

    int second_line_count = nb_player - first_line_count;
    int is_second_line = (i_player >= first_line_count);
    int line_count = is_second_line ? second_line_count : first_line_count;
    int line_index = is_second_line ? (i_player - first_line_count) : i_player;

    *out_y = is_second_line ? (base_y - spacing_y) : base_y;

    switch (line_count) {
        case 1:
            *out_x = base_x;
            break;
        case 2:
            if (is_second_line) {
                *out_x = base_x + ((line_index == 0) ? -spacing_x/2 : spacing_x/2); // Pour les 2 joueurs de la deuxième ligne : cas avec 5 joueurs.
            } else {
                *out_x = base_x + ((line_index == 0) ? -spacing_x : spacing_x);
            }
            break;
        case 3:
            *out_x = base_x + (line_index - 1) * spacing_x;
            break;
        case 4:
            *out_x = base_x + (line_index * spacing_x) - (int)(1.5f * spacing_x);
            break;
        default:
            return 0;
    }

    return 1;
}

static void render_team_member_in_panel(AppContext* context, Window* panel, SDL_Texture* icon, Text* txt, const char* name, int nb_player, int i_player, int base_y) {
    if (!context || !panel || !icon || !txt) return;

    int pos_x = 0;
    int text_y = base_y;
    if (!compute_panel_member_position(nb_player, i_player, 0, base_y, &pos_x, &text_y)) return;

    window_display_image(context->renderer, panel, icon, pos_x, text_y + 25, 0.20f, 0, SDL_FLIP_NONE, 1, 255);

    update_text(context, txt, name ? name : "???");
    window_place_text(panel, txt, pos_x, text_y);
    display_text(context, txt);
}

static void add_member_to_panel_group(Text** player_texts, int* player_index, TeamPanelMember* spies, int* spy_count, TeamPanelMember* agents, int* agent_count, UserRole role, const char* name) {
    if (!player_texts || !player_index || !spies || !spy_count || !agents || !agent_count) return;
    if (*player_index >= MAX_TEAM_PLAYERS) return;

    Text* txt = player_texts[*player_index];
    (*player_index)++;
    if (!txt) return;

    const char* safe_name = name ? name : "???";
    if (role == ROLE_SPY) {
        if (*spy_count >= MAX_TEAM_PLAYERS) return;
        spies[*spy_count].txt = txt;
        spies[*spy_count].name = safe_name;
        (*spy_count)++;
    } else {
        if (*agent_count >= MAX_TEAM_PLAYERS) return;
        agents[*agent_count].txt = txt;
        agents[*agent_count].name = safe_name;
        (*agent_count)++;
    }
}

static void render_team_panel_content(AppContext* context, Window* panel, Team team, SDL_Texture* icon, Text* txt_spy_label, Text* txt_agents_label, Text** player_texts, int* player_index) {
    if (!context || !panel || !txt_spy_label || !txt_agents_label || !player_texts || !player_index) return;

    const int SPY_BASE_Y = -85;
    const int AGENT_BASE_Y = 65;

    TeamPanelMember spy_members[MAX_TEAM_PLAYERS] = {0};
    TeamPanelMember agent_members[MAX_TEAM_PLAYERS] = {0};
    int spy_count = 0;
    int agent_count = 0;

    update_text(context, txt_spy_label, "Espions :");
    window_place_text(panel, txt_spy_label, 0, -25);
    display_text(context, txt_spy_label);

    update_text(context, txt_agents_label, "Agents :");
    window_place_text(panel, txt_agents_label, 0, 125); // Ecart de 150 entre la position de l'espion et des agents
    display_text(context, txt_agents_label);

    *player_index = 0;

    if (!context->lobby) return;

    if (context->player_team == team) {
        const char* local_name = context->player_name ? context->player_name : "Moi";
        add_member_to_panel_group(player_texts, player_index, spy_members, &spy_count, agent_members, &agent_count, context->player_role, local_name);
    }

    for (int i = 0; i < context->lobby->nb_players && i < MAX_USERS; i++) {
        User* u = context->lobby->users[i];
        if (!u || u->team != team) continue;

        add_member_to_panel_group(player_texts, player_index, spy_members, &spy_count, agent_members, &agent_count, u->role, u->name ? u->name : "???");
    }

    for (int i = 0; i < spy_count; i++) {
        render_team_member_in_panel(context, panel, icon, spy_members[i].txt, spy_members[i].name, spy_count, i, SPY_BASE_Y);
    }

    for (int i = 0; i < agent_count; i++) {
        render_team_member_in_panel(context, panel, icon, agent_members[i].txt, agent_members[i].name, agent_count, i, AGENT_BASE_Y);
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

static void game_render_team_history(AppContext* context, Window* history_window, const History* history, Text** history_texts) {
    if (!context || !history_window || !history || !history_texts) return;

    char lines[HISTORY_MAX_LINES][HISTORY_LINE_SIZE] = {{0}};
    int total_lines = history_build_lines(history, lines, HISTORY_MAX_LINES);

    int max_scroll_offset = total_lines - HISTORY_VISIBLE_LINES;
    if (max_scroll_offset < 0) max_scroll_offset = 0;

    window_edit_cfg(history_window, WIN_CFG_SCROLL_MIN, 0);
    window_edit_cfg(history_window, WIN_CFG_SCROLL_MAX, max_scroll_offset);

    int scroll_offset = history_window->cfg ? history_window->cfg->scroll_offset : 0;
    const int visible_lines = (total_lines < HISTORY_VISIBLE_LINES) ? total_lines : HISTORY_VISIBLE_LINES;
    int start_index = total_lines - visible_lines - scroll_offset;
    if (start_index < 0) start_index = 0;

    const int line_gap = 20;
    const int left_padding = 8;
    const int top_padding = 10;
    int top_line_y = (history_window->cfg->h / 2) - history_window->cfg->titlebar_h - top_padding;

    SDL_Rect history_clip = {0};
    int has_clip = (window_get_scrollable_zone_rect(history_window, &history_clip) == EXIT_SUCCESS);
    if (has_clip) {
        SDL_RenderSetClipRect(context->renderer, &history_clip);
    }

    for (int i = 0; i < HISTORY_VISIBLE_LINES; i++) {
        Text* txt = history_texts[i];
        if (!txt) continue;

        if (i >= visible_lines || (start_index + i) >= total_lines) {
            update_text(context, txt, " ");
            continue;
        }

        const char* line = lines[start_index + i];
        update_text(context, txt, line);

        int text_w = 0;
        if (txt->texture) {
            SDL_QueryTexture(txt->texture, NULL, NULL, &text_w, NULL);
        }

        int rel_x = -(history_window->cfg->w / 2) + left_padding + (text_w / 2);
        int rel_y = top_line_y - (i * line_gap);
        window_place_text(history_window, txt, rel_x, rel_y);
        display_text(context, txt);
    }

    if (has_clip) {
        SDL_RenderSetClipRect(context->renderer, NULL);
    }
}

void game_display(AppContext * context) {

    if (!audio_is_playing(MUSIC_GAME)) {
        audio_play_with_fade(MUSIC_GAME, -1, 1500, AUDIO_FADE_IN_BY_VOLUME, NULL);
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
        if (context->lobby && context->lobby->game) {
            game_render_team_history(context, history_window_blue, &context->lobby->game->blue_history, txt_history_blue_lines);
        }
    }
    if (history_window_red) {
        window_render(context->renderer, history_window_red);
        if (context->lobby && context->lobby->game) {
            game_render_team_history(context, history_window_red, &context->lobby->game->red_history, txt_history_red_lines);
        }
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

        if (context->player_role == ROLE_SPY && my_turn(context)) {
            if (hint_input) {
                window_place_input(hint_window, hint_input, -70, -15);
                input_render(context->renderer, hint_input);
            }
            if (hint_count_input) {
                window_place_input(hint_window, hint_count_input, 215, -15);
                input_render(context->renderer, hint_count_input);
            }
            if (btn_hint_submit) {
                window_place_button(hint_window, btn_hint_submit, 285, -15);
                button_render(context->renderer, btn_hint_submit);
            }
        } else {
            /* Affichage du tour et de l'indice si disponible */
            int has_hint = (context->lobby->game->current_hint[0] != '\0' && context->lobby->game->current_hint_count > 0);
            
            if (has_hint && (
                (context->lobby->game->state == GAMESTATE_TURN_BLUE_AGENT && (context->player_team == TEAM_RED || context->player_role == ROLE_AGENT)) ||
                (context->lobby->game->state == GAMESTATE_TURN_RED_AGENT && (context->player_team == TEAM_BLUE || context->player_role == ROLE_AGENT))
                )
            ) {
                /* Affichage de l'indice en grand */
                char hint_text[128];
                format_to(hint_text, sizeof(hint_text), "%s (%d)", 
                    context->lobby->game->current_hint, 
                    context->lobby->game->current_hint_count);
                update_text(context, txt_hint_display, hint_text);
                update_text_color(context, txt_hint_display, COL_WHITE);
                window_place_text(hint_window, txt_hint_display, 0, -16);
                window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)"L'indice est :");
                window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&color_text);
                display_text(context, txt_hint_display);

                if (context->player_role == ROLE_AGENT && my_turn(context)) {
                    if (btn_hint_submit) {
                        window_place_button(hint_window, btn_hint_submit, 285, -15);
                        button_render(context->renderer, btn_hint_submit);
                    }
                }
            } else if (context->lobby->game->state == GAMESTATE_ENDED) {
                // Affichage du message de fin de partie
                char hint_text[128];
                format_to(hint_text, sizeof(hint_text), "L'équipe %s a gagné !", context->lobby->game->winner == TEAM_BLUE ? "bleue" : "rouge");
                update_text(context, txt_hint_display, hint_text);
                update_text_color(context, txt_hint_display, COL_WHITE);
                window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)"La partie est terminée !");
                window_place_button(hint_window, btn_return_lobby, 375, 20);
                button_render(context->renderer, btn_return_lobby);

                display_text(context, txt_hint_display);
            } else {
                /* Affichage du tour seulement */
                update_text(context, txt_turn_label, turn_text);
                update_text_color(context, txt_turn_label, color_text);
                window_place_text(hint_window, txt_turn_label, 0, -16);
                display_text(context, txt_turn_label);
            }
        }
    }
    if (chat_window) {
        window_render(context->renderer, chat_window);
        chat_render_messages(context, chat_window, txt_chat_messages, CHAT_VISIBLE_LINES);
        if (chat_input) {
            window_place_input(chat_window, chat_input, 0, -75);
            input_render(context->renderer, chat_input);
        }
    }

    game_render_team_windows(context);
}


int game_free() {
    if (btn_quit_game) { button_destroy(btn_quit_game); btn_quit_game = NULL; }
    if (btn_hint_submit) { button_destroy(btn_hint_submit); btn_hint_submit = NULL; }
    if (btn_return_lobby) { button_destroy(btn_return_lobby); btn_return_lobby = NULL; }
    if (hint_input) { input_destroy(hint_input); hint_input = NULL; }
    if (hint_count_input) { input_destroy(hint_count_input); hint_count_input = NULL; }
    if (chat_input) { input_destroy(chat_input); chat_input = NULL; }
    if (blue_panel) { window_destroy(blue_panel); blue_panel = NULL; }
    if (red_panel) { window_destroy(red_panel); red_panel = NULL; }
    if (history_window_blue) { window_destroy(history_window_blue); history_window_blue = NULL; }
    if (history_window_red) { window_destroy(history_window_red); history_window_red = NULL; }
    if (hint_window) { window_destroy(hint_window); hint_window = NULL; }
    if (chat_window) { window_destroy(chat_window); chat_window = NULL; }

    /* Libération des textes optimisés */
    destroy_text(txt_team_blue_title); txt_team_blue_title = NULL;
    destroy_text(txt_team_red_title); txt_team_red_title = NULL;
    destroy_text(txt_blue_spy_label); txt_blue_spy_label = NULL;
    destroy_text(txt_blue_agents_label); txt_blue_agents_label = NULL;
    destroy_text(txt_red_spy_label); txt_red_spy_label = NULL;
    destroy_text(txt_red_agents_label); txt_red_agents_label = NULL;
    destroy_text(txt_turn_label); txt_turn_label = NULL;
    destroy_text(txt_hint_display); txt_hint_display = NULL;

    for (int i = 0; i < MAX_TEAM_PLAYERS; i++) {
        destroy_text(txt_blue_players[i]); txt_blue_players[i] = NULL;
        destroy_text(txt_red_players[i]); txt_red_players[i] = NULL;
    }

    for (int i = 0; i < CHAT_VISIBLE_LINES; i++) {
        destroy_text(txt_chat_messages[i]); txt_chat_messages[i] = NULL;
    }

    for (int i = 0; i < HISTORY_VISIBLE_LINES; i++) {
        destroy_text(txt_history_blue_lines[i]); txt_history_blue_lines[i] = NULL;
        destroy_text(txt_history_red_lines[i]); txt_history_red_lines[i] = NULL;
    }
    
    return EXIT_SUCCESS;
}
