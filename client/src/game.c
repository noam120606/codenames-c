#include "../lib/all.h"

/* Ressources UI du module jeu */
static Button* btn_quit_game = NULL;
static Button* btn_hint_submit = NULL;
static Button* btn_return_lobby = NULL;

static Input* hint_input = NULL;
static Input* hint_count_input = NULL;
static Input* chat_input = NULL;

static const char* HINT_INPUT_PLACEHOLDERS[] = {"Entrez un mot indice"};
static const char* HINT_COUNT_INPUT_PLACEHOLDERS[] = {"1","2","3"};
static const char* CHAT_INPUT_PLACEHOLDERS[] = {"Chattez ici ..."};

static Window* blue_panel = NULL;
static Window* red_panel = NULL;
static Window* history_window_blue = NULL;
static Window* history_window_red = NULL;
static Window* hint_window = NULL;
static Window* chat_window = NULL;

/* Utilisation des icônes déjà chargées dans lobby.c */
extern SDL_Texture* player_icon_red;
extern SDL_Texture* player_icon_blue;

/* Couleurs pour les panneaux d'équipe */
static const SDL_Color TEAM_BLUE_COLOR = {50, 80, 150, 200};
static const SDL_Color TEAM_RED_COLOR = {150, 50, 50, 200};
static const SDL_Color HISTORY_WORD_BLUE_COLOR = {110, 160, 255, 255};
static const SDL_Color HISTORY_WORD_RED_COLOR = {255, 120, 120, 255};
static const SDL_Color HISTORY_WORD_BLACK_COLOR = {170, 170, 170, 255};
static const SDL_Color HISTORY_WORD_NONE_COLOR = {224, 198, 149, 255};

/* Textes optimisés pour le jeu */
static Text* txt_team_blue_title = NULL;
static Text* txt_team_red_title = NULL;
static Text* txt_blue_spy_label = NULL;
static Text* txt_blue_agents_label = NULL;
static Text* txt_red_spy_label = NULL;
static Text* txt_red_agents_label = NULL;

static Text* txt_turn_label = NULL;
static Text* txt_hint_display = NULL;
static Text* txt_endGame = NULL;

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
#define HISTORY_VISIBLE_LINES 13
static Text* txt_history_blue_lines[HISTORY_VISIBLE_LINES] = {NULL};
static Text* txt_history_red_lines[HISTORY_VISIBLE_LINES] = {NULL};
static Text* txt_history_blue_prefix_lines[HISTORY_VISIBLE_LINES] = {NULL};
static Text* txt_history_red_prefix_lines[HISTORY_VISIBLE_LINES] = {NULL};
static HistoryWrapCache history_cache_blue = {0};
static HistoryWrapCache history_cache_red = {0};
static const char GAME_HINTBAR_DEFAULT_TITLE[] = "Informations de partie";

static Game* game_get_active_game(AppContext* context) {
    if (!context || !context->lobby || !context->lobby->game) return NULL;
    return context->lobby->game;
}

static GameHintBarState* game_get_hint_bar_state(AppContext* context) {
    Game* game = game_get_active_game(context);
    return game ? &game->hint_bar : NULL;
}

static int game_has_active_match(const AppContext* context) {
    return context && context->lobby && context->lobby->game;
}

static int game_has_hint(const Game* game) {
    return game && game->current_hint[0] != '\0' && game->current_hint_count > 0;
}

static int game_should_show_hint_to_player(const AppContext* context, const Game* game) {
    if (!context || !game || !game_has_hint(game)) return 0;

    return (
        (game->state == GAMESTATE_TURN_BLUE_AGENT && (context->player_team == TEAM_RED || context->player_role == ROLE_AGENT)) ||
        (game->state == GAMESTATE_TURN_RED_AGENT && (context->player_team == TEAM_BLUE || context->player_role == ROLE_AGENT))
    );
}

static void game_hint_bar_write_message(GameHintBarMessage* message, const char* text, SDL_Color color, int priority, Uint32 expire_at_ms, int active) {
    if (!message) return;

    message->color = color;
    message->priority = priority;
    message->expire_at_ms = expire_at_ms;
    message->active = active;

    if (!text || text[0] == '\0') {
        message->text[0] = '\0';
        message->active = 0;
        return;
    }

    strncpy(message->text, text, GAME_HINTBAR_TEXT_LEN - 1);
    message->text[GAME_HINTBAR_TEXT_LEN - 1] = '\0';
}

void game_hint_bar_set_context(AppContext* context, const char* text, SDL_Color color) {
    GameHintBarState* state = game_get_hint_bar_state(context);
    if (!state) return;

    const char* safe_text = (text && text[0] != '\0') ? text : "";
    if (
        state->context.active &&
        strcmp(state->context.text, safe_text) == 0 &&
        state->context.color.r == color.r &&
        state->context.color.g == color.g &&
        state->context.color.b == color.b &&
        state->context.color.a == color.a &&
        state->context.priority == GAME_HINTBAR_PRIORITY_CONTEXT
    ) {
        return;
    }

    game_hint_bar_write_message(
        &state->context,
        safe_text,
        color,
        GAME_HINTBAR_PRIORITY_CONTEXT,
        0,
        1
    );
}

static int game_hint_bar_feedback_expired(const GameHintBarMessage* feedback, Uint32 now) {
    if (!feedback || !feedback->active) return 1;
    if (feedback->expire_at_ms == 0) return 0;
    return SDL_TICKS_PASSED(now, feedback->expire_at_ms);
}

void game_hint_bar_set_feedback(AppContext* context, const char* text, SDL_Color color, int priority, Uint32 duration_ms) {
    GameHintBarState* state = game_get_hint_bar_state(context);
    if (!state || !text || text[0] == '\0') return;

    if (priority < GAME_HINTBAR_PRIORITY_CONTEXT) {
        priority = GAME_HINTBAR_PRIORITY_CONTEXT;
    }

    Uint32 now = SDL_GetTicks();
    if (
        state->feedback.active &&
        !game_hint_bar_feedback_expired(&state->feedback, now) &&
        state->feedback.priority >= priority
    ) {
        return;
    }

    Uint32 expire_at_ms = (duration_ms > 0) ? (now + duration_ms) : 0;
    game_hint_bar_write_message(&state->feedback, text, color, priority, expire_at_ms, 1);
}

void game_hint_bar_clear_feedback(AppContext* context) {
    GameHintBarState* state = game_get_hint_bar_state(context);
    if (!state) return;

    state->feedback.text[0] = '\0';
    state->feedback.active = 0;
    state->feedback.priority = GAME_HINTBAR_PRIORITY_CONTEXT;
    state->feedback.expire_at_ms = 0;
}

static int game_hint_bar_colors_equal(SDL_Color left, SDL_Color right) {
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}

static const GameHintBarMessage* game_hint_bar_resolve_message(GameHintBarState* state) {
    if (!state) return NULL;

    Uint32 now = SDL_GetTicks();
    if (game_hint_bar_feedback_expired(&state->feedback, now)) {
        state->feedback.active = 0;
    }

    if (state->feedback.active && state->feedback.text[0] != '\0') {
        if (!state->context.active || state->feedback.priority >= state->context.priority) {
            return &state->feedback;
        }
    }

    if (state->context.active && state->context.text[0] != '\0') {
        return &state->context;
    }

    return NULL;
}

static void game_hint_bar_update_context(AppContext* context) {
    Game* game = game_get_active_game(context);
    if (!game) return;

    const char* context_title = "Phase de preparation";
    SDL_Color context_color = COL_GRAY;

    switch (game->state) {
        case GAMESTATE_TURN_BLUE_SPY:
            context_title = "Tour de l'espion bleu";
            context_color = TEAM_BLUE_COLOR;
            break;
        case GAMESTATE_TURN_BLUE_AGENT:
            context_title = "Tour de l'agent bleu";
            context_color = TEAM_BLUE_COLOR;
            break;
        case GAMESTATE_TURN_RED_SPY:
            context_title = "Tour de l'espion rouge";
            context_color = TEAM_RED_COLOR;
            break;
        case GAMESTATE_TURN_RED_AGENT:
            context_title = "Tour de l'agent rouge";
            context_color = TEAM_RED_COLOR;
            break;
        case GAMESTATE_ENDED:
            context_title = "La partie est terminee !";
            if (game->winner == TEAM_BLUE) {
                context_color = TEAM_BLUE_COLOR;
            } else if (game->winner == TEAM_RED) {
                context_color = TEAM_RED_COLOR;
            } else {
                context_color = COL_GRAY;
            }
            break;
        case GAMESTATE_WAITING:
        default:
            context_title = "Phase de preparation";
            context_color = COL_GRAY;
            break;
    }

    if (game_should_show_hint_to_player(context, game)) {
        context_title = "L'indice est :";
        context_color = (game->state == GAMESTATE_TURN_BLUE_AGENT) ? TEAM_BLUE_COLOR : TEAM_RED_COLOR;
    }

    game_hint_bar_set_context(context, context_title, context_color);
}

static void game_hint_bar_apply(AppContext* context) {
    if (!hint_window) return;

    GameHintBarState* state = game_get_hint_bar_state(context);
    if (!state) return;

    const GameHintBarMessage* active_message = game_hint_bar_resolve_message(state);
    const char* title = GAME_HINTBAR_DEFAULT_TITLE;
    SDL_Color titlebar_color = COL_GRAY;

    if (active_message) {
        if (active_message->text[0] != '\0') {
            title = active_message->text;
        }
        titlebar_color = active_message->color;
    }

    if (!state->applied_valid || strcmp(state->applied_text, title) != 0) {
        window_edit_cfg(hint_window, WIN_CFG_TITLE, (intptr_t)title);
        strncpy(state->applied_text, title, GAME_HINTBAR_TEXT_LEN - 1);
        state->applied_text[GAME_HINTBAR_TEXT_LEN - 1] = '\0';
    }

    if (!state->applied_valid || !game_hint_bar_colors_equal(state->applied_color, titlebar_color)) {
        window_edit_cfg(hint_window, WIN_CFG_TITLEBAR_COLOR, (intptr_t)&titlebar_color);
        state->applied_color = titlebar_color;
    }

    state->applied_valid = 1;
}

static void hint_on_submit(AppContext* context, const char* text) {
    (void)context;
    (void)text;
    if (hint_count_input) {
        hint_count_input->cfg->focused = 1;
        hint_count_input->cfg->cursor_pos = strlen(hint_count_input->cfg->text);
    }
    // Si le joueur presse encore la touche Entrée ou Entrée du numpad alors il reste sur le champ de saisie du nombre d'indices pour l'inviter à saisir un nombre d'indices après avoir saisi un mot indice.
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYUP && (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)) {
            if (hint_count_input) {
                hint_count_input->cfg->focused = 1;
                hint_count_input->cfg->cursor_pos = strlen(hint_count_input->cfg->text);
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
    if (!context || !button) return BTN_NONE;

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
        game_hint_bar_clear_feedback(context);

        return BTN_GAME_RETURN_LOBBY;
    }

    if (button == btn_hint_submit) {

        if (context->player_role == ROLE_AGENT) {
            History* history = (context->player_team == TEAM_RED) ? &context->lobby->game->red_history : &context->lobby->game->blue_history;
            Turn* last_turn = (history->turn_count > 0) ? &history->turns[history->turn_count - 1] : NULL;
            if (!last_turn || last_turn->revealed_word_count < context->lobby->game->current_hint_count) {
                printf("Nombre de mots minimum non atteint pour s'arreter\n");
                char title[GAME_HINTBAR_TEXT_LEN];
                format_to(title, sizeof(title), "Sélectionnez au moins %d mot(s)", context->lobby->game->current_hint_count);
                game_hint_bar_set_feedback(
                    context,
                    title,
                    COL_RED,
                    GAME_HINTBAR_PRIORITY_ERROR,
                    GAME_HINTBAR_FEEDBACK_ERROR_MS
                );
                return BTN_NONE;
            }
            char msg[64];
            const char* agent_name = (context->player_name && context->player_name[0] != '\0') ? context->player_name : "Unknown";
            format_to(msg, sizeof(msg), "%d -1 %s", MSG_GUESS_CARD, agent_name);
            send_tcp(context->sock, msg);
            return BTN_NONE;
        }

        char* hint = hint_input->cfg->text;
        int nb_hint = (int)atoi(hint_count_input->cfg->text);
        int valid = valid_hint(hint, context->lobby->game->cards);
        
        
        char title[GAME_HINTBAR_TEXT_LEN];
        /* Envoi de l'indice au serveur */
        // On stocke cette chaine : "Vous avez soumis le mot : " et on lui ajoute "text" et "en nb_hint"
        if (hint && nb_hint && valid) {
            format_to(title, sizeof(title), "Vous avez soumis le mot : %s en %d", hint ? hint : "", nb_hint);
            game_hint_bar_set_feedback(
                context,
                title,
                COL_DARK_GREEN,
                GAME_HINTBAR_PRIORITY_INFO,
                GAME_HINTBAR_FEEDBACK_SUCCESS_MS
            );
            char msg[64];
            format_to(msg, sizeof(msg), "%d %d %s", MSG_SUBMIT_HINT, nb_hint, hint); // Pas besoin de passer l'id du joueur car le serveur peut l'identifier grâce au socket
            send_tcp(context->sock, msg);
            clear_hint_inputs();
        } else if(hint == NULL || strlen(hint) == 0) {
            game_hint_bar_set_feedback(
                context,
                "Vous n'avez pas saisi de mot",
                COL_RED,
                GAME_HINTBAR_PRIORITY_ERROR,
                GAME_HINTBAR_FEEDBACK_ERROR_MS
            );
            printf("Invalid hint submitted: %s\n", hint ? hint : "");
        } else if(nb_hint <= 0) {
            game_hint_bar_set_feedback(
                context,
                "Vous n'avez pas saisi de nombre d'indices",
                COL_RED,
                GAME_HINTBAR_PRIORITY_ERROR,
                GAME_HINTBAR_FEEDBACK_ERROR_MS
            );
            printf("No hint count submitted\n");
        } else {
            format_to(title, sizeof(title), "Le mot : %s est invalide", hint ? hint : "");
            game_hint_bar_set_feedback(
                context,
                title,
                COL_RED,
                GAME_HINTBAR_PRIORITY_ERROR,
                GAME_HINTBAR_FEEDBACK_ERROR_MS
            );
            printf("Invalid hint submitted: %s\n", hint ? hint : "");
        }

        return BTN_NONE;
    }

    if (button == btn_return_lobby) {
        context->app_state = APP_STATE_LOBBY;
        printf("Returned to lobby\n");
        game_struct_free(context);
        return BTN_NONE;
    }

    return BTN_NONE;
}

int my_turn(AppContext* context) {
    if (!game_has_active_match(context)) return 0;

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
    Game* game = game_get_active_game(context);
    int has_game = (game != NULL);
    int is_my_turn = has_game ? my_turn(context) : 0;
    int show_spy_hint_controls = (context->player_role == ROLE_SPY && is_my_turn);
    int show_hint_for_agents_or_opponents = has_game && game_should_show_hint_to_player(context, game);
    int show_agent_hint_button = (show_hint_for_agents_or_opponents && context->player_role == ROLE_AGENT && is_my_turn);
    int show_return_lobby_button = (has_game && game->state == GAMESTATE_ENDED);

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

    history_wrap_cache_invalidate(&history_cache_blue);
    history_wrap_cache_invalidate(&history_cache_red);

    ButtonConfig* cfg_btn_quit_game = button_config_init();
    if (cfg_btn_quit_game) {
        cfg_btn_quit_game->x = -775;
        cfg_btn_quit_game->y = 450;
        cfg_btn_quit_game->h = 64;
        cfg_btn_quit_game->font_path = FONT_LARABIE;
        cfg_btn_quit_game->color = COL_WHITE;
        cfg_btn_quit_game->text = "Quitter la partie";
        cfg_btn_quit_game->callback = game_button_click;
        btn_quit_game = button_create(context->renderer, BTN_GAME_RETURN_LOBBY, cfg_btn_quit_game);
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
        btn_hint_submit = button_create(context->renderer, BTN_GAME_VALIDATE_HINT, cfg_btn_hint_submit);
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
        btn_return_lobby = button_create(context->renderer, BTN_GAME_RETURN_LOBBY, cfg_btn_return_lobby);
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
        cfg_hint_count_input->allowed_pattern = "^[0-9]$";
        cfg_hint_count_input->submit_pattern = "^[0-9]{1}$";
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
        create_text_config(FONT_LARABIE, 32, COL_WHITE, 0, 0, 0, 255));

    txt_endGame = init_text(context, "", 
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

        txt_history_blue_prefix_lines[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 18, (SDL_Color){0, 200, 0, 255}, 0, 0, 0, 255));
        if (!txt_history_blue_prefix_lines[i]) loading_fails++;

        txt_history_red_lines[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 18, COL_WHITE, 0, 0, 0, 255));
        if (!txt_history_red_lines[i]) loading_fails++;

        txt_history_red_prefix_lines[i] = init_text(context, " ",
            create_text_config(FONT_NOTO, 18, (SDL_Color){0, 200, 0, 255}, 0, 0, 0, 255));
        if (!txt_history_red_prefix_lines[i]) loading_fails++;
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
        blue_panel = window_create(WINDOW_GAME_BLUE_PANEL, cfg_blue_panel);
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
        red_panel = window_create(WINDOW_GAME_RED_PANEL, cfg_red_panel);
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
        history_window_blue = window_create(WINDOW_GAME_HISTORY_BLUE, cfg_history_blue);
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
        history_window_red = window_create(WINDOW_GAME_HISTORY_RED, cfg_history_red);
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
        cfg_hint_window->title = (char*)GAME_HINTBAR_DEFAULT_TITLE;
        cfg_hint_window->titlebar_color = COL_GRAY;
        cfg_hint_window->bg_color = (SDL_Color){20, 20, 20, 220};
        hint_window = window_create(WINDOW_GAME_HINT, cfg_hint_window);
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
        chat_window = window_create(WINDOW_GAME_CHAT, cfg_chat_window);
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
    int local_player_injected = 0;

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
        local_player_injected = 1;
    }

    for (int i = 0; i < MAX_USERS; i++) {
        User* u = context->lobby->users[i];
        if (!u || u->team != team) continue;
        if (local_player_injected && context->player_id >= 0 && u->id == context->player_id) continue;

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

static int game_colors_equal(SDL_Color left, SDL_Color right) {
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}

static SDL_Color game_history_word_color(Team word_team) {
    switch (word_team) {
        case TEAM_RED:
            return HISTORY_WORD_RED_COLOR;
        case TEAM_BLUE:
            return HISTORY_WORD_BLUE_COLOR;
        case TEAM_BLACK:
            return HISTORY_WORD_BLACK_COLOR;
        case TEAM_NONE:
            return HISTORY_WORD_NONE_COLOR;
        default:
            return COL_WHITE;
    }
}

static void game_render_team_history(AppContext* context, Window* history_window, const History* history, Text** history_texts, Text** history_prefix_texts, HistoryWrapCache* history_cache) {
    if (!context || !history_window || !history || !history_texts || !history_prefix_texts || !history_window->cfg || !history_cache) return;

    const int line_gap = 20;
    const int left_padding = 8;
    const int right_padding = 8;
    const int top_padding = 10;
    int max_text_width = history_window->cfg->w - left_padding - right_padding;
    if (max_text_width < 32) max_text_width = 32;

    const char* font_path = NULL;
    int font_size = 0;
    if (history_texts[0]) {
        font_path = history_texts[0]->cfg.font_path;
        font_size = history_texts[0]->cfg.font_size;
    }

    int total_lines = history_build_wrapped_lines_cached(
        history,
        font_path,
        font_size,
        max_text_width,
        history_cache
    );
    const char (*lines)[HISTORY_LINE_SIZE] = history_cache->lines;

    int max_scroll_offset = total_lines - HISTORY_VISIBLE_LINES;
    if (max_scroll_offset < 0) max_scroll_offset = 0;

    if (history_window->cfg->scroll_min != 0) {
        window_edit_cfg(history_window, WIN_CFG_SCROLL_MIN, 0);
    }
    if (history_window->cfg->scroll_max != max_scroll_offset) {
        window_edit_cfg(history_window, WIN_CFG_SCROLL_MAX, max_scroll_offset);
    }

    int scroll_offset = history_window->cfg->scroll_offset;
    if (scroll_offset < 0) scroll_offset = 0;
    if (scroll_offset > max_scroll_offset) scroll_offset = max_scroll_offset;

    const int visible_lines = (total_lines < HISTORY_VISIBLE_LINES) ? total_lines : HISTORY_VISIBLE_LINES;
    int start_index = total_lines - visible_lines - scroll_offset;
    if (start_index < 0) start_index = 0;

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
            if (!game_colors_equal(txt->cfg.color, COL_WHITE)) {
                update_text_color(context, txt, COL_WHITE);
            }
            if (!txt->content || strcmp(txt->content, " ") != 0) {
                update_text(context, txt, " ");
            }
            continue;
        }

        const char* line = lines[start_index + i];
        const char* safe_line = line ? line : " ";
        const char* white_line = safe_line;
        int white_left_offset = 0;
        Text* turn_prefix_text = history_prefix_texts[i];
        Team word_team = history_cache->line_word_teams[start_index + i];
        int has_word_team = history_cache->line_has_revealed_word_team[start_index + i];
        SDL_Color text_color = has_word_team ? game_history_word_color(word_team) : COL_WHITE;

        if (!has_word_team && turn_prefix_text) {
            char turn_prefix[HISTORY_LINE_SIZE];
            int prefix_len = 0;
            if (history_extract_turn_prefix(safe_line, turn_prefix, HISTORY_LINE_SIZE, &prefix_len) == EXIT_SUCCESS) {
                white_line = safe_line + prefix_len;
                if (!white_line || white_line[0] == '\0') {
                    white_line = " ";
                }
                white_left_offset = history_render_turn_prefix(
                    context,
                    history_window,
                    left_padding,
                    turn_prefix,
                    top_line_y - (i * line_gap),
                    turn_prefix_text
                );
            }
        }

        if (!game_colors_equal(txt->cfg.color, text_color)) {
            update_text_color(context, txt, text_color);
        }

        if (!txt->content || strcmp(txt->content, white_line) != 0) {
            update_text(context, txt, white_line);
        }

        int text_w = history_text_width(txt);

        int rel_x = history_left_anchored_rel_x_with_offset(history_window, text_w, left_padding, white_left_offset);
        int rel_y = top_line_y - (i * line_gap);
        window_place_text(history_window, txt, rel_x, rel_y);
        display_text(context, txt);
    }

    if (has_clip) {
        SDL_RenderSetClipRect(context->renderer, NULL);
    }
}

void game_display(AppContext * context) {
    if (!context) return;

    Game* game = game_get_active_game(context);

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
        if (game) {
            game_render_team_history(context, history_window_blue, &game->blue_history, txt_history_blue_lines, txt_history_blue_prefix_lines, &history_cache_blue);
        }
    }
    if (history_window_red) {
        window_render(context->renderer, history_window_red);
        if (game) {
            game_render_team_history(context, history_window_red, &game->red_history, txt_history_red_lines, txt_history_red_prefix_lines, &history_cache_red);
        }
    }

    if (hint_window) {
        game_hint_bar_update_context(context);
        game_hint_bar_apply(context);
        window_render(context->renderer, hint_window);

        if (game) {
            const char* turn_text = "";
            SDL_Color color_text = COL_GRAY;

            switch (game->state) {
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
                int has_hint = game_has_hint(game);

                if (has_hint && game_should_show_hint_to_player(context, game)) {
                    /* Affichage de l'indice en grand */
                    char hint_text[128];
                    format_to(hint_text, sizeof(hint_text), "%s (%d)",
                        game->current_hint,
                        game->current_hint_count);
                    update_text(context, txt_hint_display, hint_text);
                    update_text_color(context, txt_hint_display, COL_WHITE);
                    window_place_text(hint_window, txt_hint_display, 0, -16);
                    display_text(context, txt_hint_display);

                    if (context->player_role == ROLE_AGENT && my_turn(context)) {
                        if (btn_hint_submit) {
                            window_place_button(hint_window, btn_hint_submit, 285, -15);
                            button_render(context->renderer, btn_hint_submit);
                        }
                    }
                } else if (game->state == GAMESTATE_ENDED) {
                    // Affichage du message de fin de partie
                    char endGame_text[128];
                    format_to(endGame_text, sizeof(endGame_text), "L'équipe %s a gagné !", game->winner == TEAM_BLUE ? "bleue" : "rouge");
                    update_text(context, txt_hint_display, endGame_text);
                    if (game->winner == TEAM_BLUE) {
                        update_text_color(context, txt_hint_display, TEAM_BLUE_COLOR);
                    } else {
                        update_text_color(context, txt_hint_display, TEAM_RED_COLOR);
                    }
                    window_place_text(hint_window, txt_hint_display, 0, 0);
                    display_text(context, txt_hint_display);
                    // Affichage des noms des joueurs gagnants juste en dessous du message de fin de partie
                    char winners_text[256];
                    int winners_text_offset = format_to(winners_text, sizeof(winners_text), "Bravo aux gagnants : ");
                    if (winners_text_offset < 0) winners_text_offset = 0;
                    int winner_count = 0;
                    int local_winner_already_listed = 0;
                    for (int i = 0; i < MAX_USERS; i++) {
                        User* u = context->lobby->users[i];
                        if (!u || u->team != game->winner) continue;

                        if (context->player_name && u->name && strcmp(context->player_name, u->name) == 0) {
                            local_winner_already_listed = 1;
                        }

                        if (winners_text_offset < (int)sizeof(winners_text) - 1) {
                            size_t remaining = sizeof(winners_text) - (size_t)winners_text_offset;
                            int written = format_to(
                                winners_text + winners_text_offset,
                                remaining,
                                "%s%s",
                                winner_count > 0 ? " - " : "",
                                u->name ? u->name : "???"
                            );
                            if (written < 0) {
                                break;
                            }
                            if ((size_t)written >= remaining) {
                                winners_text_offset = (int)sizeof(winners_text) - 1;
                            } else {
                                winners_text_offset += written;
                            }
                        }

                        winner_count++;
                    }

                    if (
                        context->player_team == game->winner &&
                        context->player_name && context->player_name[0] != '\0' &&
                        !local_winner_already_listed
                    ) {
                        if (winners_text_offset < (int)sizeof(winners_text) - 1) {
                            size_t remaining = sizeof(winners_text) - (size_t)winners_text_offset;
                            int written = format_to(
                                winners_text + winners_text_offset,
                                remaining,
                                "%s%s",
                                winner_count > 0 ? " - " : "",
                                context->player_name
                            );
                            if (written >= 0) {
                                if ((size_t)written >= remaining) {
                                    winners_text_offset = (int)sizeof(winners_text) - 1;
                                } else {
                                    winners_text_offset += written;
                                }
                            }
                        }
                        winner_count++;
                    }

                    if (winner_count == 0) {
                        format_to(winners_text, sizeof(winners_text), "Bravo aux gagnants : ???");
                    }
                    window_place_button(hint_window, btn_return_lobby, 450, 0);
                    button_render(context->renderer, btn_return_lobby);
                    // Affichage du message des gagnants
                    update_text(context, txt_endGame, winners_text);
                    window_place_text(hint_window, txt_endGame, 0, -32);
                    display_text(context, txt_endGame);
                } else {
                    /* Affichage du tour seulement */
                    update_text(context, txt_turn_label, turn_text);
                    update_text_color(context, txt_turn_label, color_text);
                    window_place_text(hint_window, txt_turn_label, 0, -16);
                    display_text(context, txt_turn_label);
                }
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
    destroy_text(txt_endGame); txt_endGame = NULL;
    history_destroy_turn_prefix_text();

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
        destroy_text(txt_history_blue_prefix_lines[i]); txt_history_blue_prefix_lines[i] = NULL;
        destroy_text(txt_history_red_prefix_lines[i]); txt_history_red_prefix_lines[i] = NULL;
    }

    history_wrap_cache_invalidate(&history_cache_blue);
    history_wrap_cache_invalidate(&history_cache_red);
    
    return EXIT_SUCCESS;
}
