#include "../lib/all.h"

Button* btn_red_agent = NULL;
Button* btn_red_spy = NULL;
Button* btn_blue_agent = NULL;
Button* btn_blue_spy = NULL;
Button* btn_launch_game = NULL;
Button* btn_return = NULL;

Window* role_none_window = NULL;
Window* role_red_agent_window = NULL;
Window* role_red_spy_window = NULL;
Window* role_blue_agent_window = NULL;
Window* role_blue_spy_window = NULL;

Window* game_options_window = NULL;

SDL_Texture* player_icon_none = NULL;
SDL_Texture* player_icon_red = NULL;
SDL_Texture* player_icon_blue = NULL;

/* Textes optimisés pour le lobby */
static Text* txt_lobby_info = NULL;
static Text* txt_no_role_label = NULL;
#define MAX_PLAYER_TEXTS 16
static Text* txt_player_names[MAX_PLAYER_TEXTS] = {NULL};
static int current_player_text_index = 0;

typedef struct PlayerPlacementCounters {
    int nb_none;
    int nb_red_agent;
    int nb_red_spy;
    int nb_blue_agent;
    int nb_blue_spy;
    int i_none;
    int i_red_agent;
    int i_red_spy;
    int i_blue_agent;
    int i_blue_spy;
} PlayerPlacementCounters;

static void count_player_for_layout(const User* user, PlayerPlacementCounters* counters) {
    if (!user || !counters) return;

    const char* name = user->name ? user->name : "Unknown";
    switch (user->team) {
        case TEAM_RED:
            user->role == ROLE_SPY ? counters->nb_red_spy++ : counters->nb_red_agent++;
            break;
        case TEAM_BLUE:
            user->role == ROLE_SPY ? counters->nb_blue_spy++ : counters->nb_blue_agent++;
            break;
        case TEAM_NONE:
            counters->nb_none++;
            break;
        default:
            printf("Warning: User %s has invalid team %d\n", name, user->team);
            break;
    }
}

static void advance_player_layout_index(const User* user, PlayerPlacementCounters* counters) {
    if (!user || !counters) return;

    const char* name = user->name ? user->name : "Unknown";
    switch (user->team) {
        case TEAM_RED:
            user->role == ROLE_SPY ? counters->i_red_spy++ : counters->i_red_agent++;
            break;
        case TEAM_BLUE:
            user->role == ROLE_SPY ? counters->i_blue_spy++ : counters->i_blue_agent++;
            break;
        case TEAM_NONE:
            counters->i_none++;
            break;
        default:
            printf("Warning: User %s has invalid team %d\n", name, user->team);
            break;
    }
}

static int compute_player_icon_position(int nb_player, int i_player, int base_x, int base_y, int* out_x, int* out_y) {
    if (!out_x || !out_y) return 0;

    const int spacing_x = 80;
    const int spacing_y = 80;

    if (nb_player < 1 || nb_player > 8) {
        printf("Too many players for lobby display: %d\n", nb_player);
        return 0;
    }

    if (i_player < 0 || i_player >= nb_player) {
        return 0;
    }

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

    int line_count = 0;
    int line_index = 0;
    int is_second_line = (i_player >= first_line_count);

    if (is_second_line) {
        line_count = second_line_count;
        line_index = i_player - first_line_count;
        *out_y = base_y - spacing_y;
    } else {
        line_count = first_line_count;
        line_index = i_player;
        *out_y = base_y;
    }

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

/* Callback pour boutons du lobby (choix de rôle / équipe) */
static ButtonReturn lobby_button_click(AppContext* context, Button* button) {
    if (!context || !button) return BTN_RET_NONE;
    if (button == btn_red_agent) {
        if (context->player_role == ROLE_AGENT && context->player_team == TEAM_RED) {
            printf("Role didn't change\n");
            return BTN_RET_NONE;
        }
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_RED;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_AGENT, TEAM_RED);
        send_tcp(context->sock, msg);

        printf("Selected role: RED AGENT\n");
    } else if (button == btn_red_spy) {
        if (context->player_role == ROLE_SPY && context->player_team == TEAM_RED) {
            printf("Role didn't change\n");
            return BTN_RET_NONE;
        }
        context->player_role = ROLE_SPY;
        context->player_team = TEAM_RED;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_SPY, TEAM_RED);
        send_tcp(context->sock, msg);
        printf("Selected role: RED SPY\n");
    } else if (button == btn_blue_agent) {
        if (context->player_role == ROLE_AGENT && context->player_team == TEAM_BLUE) {
            printf("Role didn't change\n");
            return BTN_RET_NONE;
        }
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_BLUE;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_AGENT, TEAM_BLUE);
        send_tcp(context->sock, msg);
        printf("Selected role: BLUE AGENT\n");
    } else if (button == btn_blue_spy) {
        if (context->player_role == ROLE_SPY && context->player_team == TEAM_BLUE) {
            printf("Role didn't change\n");
            return BTN_RET_NONE;
        }
        context->player_role = ROLE_SPY;
        context->player_team = TEAM_BLUE;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_SPY, TEAM_BLUE);
        send_tcp(context->sock, msg);
        printf("Selected role: BLUE SPY\n");
    } else if (button == btn_launch_game) {
        /* Informer le serveur du lancement de la partie (si protocole / message existant) */
        char msg[16];
        format_to(msg, sizeof(msg), "%d", MSG_STARTGAME);
        send_tcp(context->sock, msg);
        printf("Launch game requested\n");
    } else if (button == btn_return) {
        /* Informer le serveur qu'on quitte le lobby */
        if (context->lobby->id >= 0 && context->sock >= 0) {
            struct_lobby_init(context->lobby, -1, "");
            char msg[16];
            format_to(msg, sizeof(msg), "%d", MSG_LEAVELOBBY);
            send_tcp(context->sock, msg);
        }
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;
        context->lobby->id = -1;

        /* Retirer le filtre audio en quittant le lobby. */
        audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_NONE, 0);
        context->app_state = APP_STATE_MENU;
        printf("Returned to menu\n");
    }
    return BTN_RET_NONE;
}

int lobby_init(AppContext* context) {

    int loading_fails = 0;

    // Chargement images
    player_icon_none = load_image(context->renderer, "assets/img/player_icons/none.png");
    if (!player_icon_none) {
        printf("Failed to load player_icon_none image\n");
        loading_fails++;
    }

    player_icon_red = load_image(context->renderer, "assets/img/player_icons/red.png");
    if (!player_icon_red) {
        printf("Failed to load player_icon_red image\n");
        loading_fails++;
    }

    player_icon_blue = load_image(context->renderer, "assets/img/player_icons/blue.png");
    if (!player_icon_blue) {
        printf("Failed to load player_icon_blue image\n");
        loading_fails++;
    }

    /* Créer les boutons de rôle/équipe via ButtonConfig */
    int height = 64;

    ButtonConfig* cfg_btn_red_agent = button_config_init();
    if (cfg_btn_red_agent) {
        cfg_btn_red_agent->x         = -400;
        cfg_btn_red_agent->y         = 260;
        cfg_btn_red_agent->h         = height;
        cfg_btn_red_agent->font_path = FONT_LARABIE;
        cfg_btn_red_agent->color     = COL_WHITE;
        cfg_btn_red_agent->text      = "Agent rouge";
        cfg_btn_red_agent->tex_path  = "assets/img/buttons/red.png";
        cfg_btn_red_agent->callback  = lobby_button_click;
        btn_red_agent = button_create(context->renderer, 0, cfg_btn_red_agent);
        free(cfg_btn_red_agent);
    } else loading_fails++;

    ButtonConfig* cfg_btn_red_spy = button_config_init();
    if (cfg_btn_red_spy) {
        cfg_btn_red_spy->x         = -400;
        cfg_btn_red_spy->y         = 0;
        cfg_btn_red_spy->h         = height;
        cfg_btn_red_spy->font_path = FONT_LARABIE;
        cfg_btn_red_spy->color     = COL_WHITE;
        cfg_btn_red_spy->text      = "Espion rouge";
        cfg_btn_red_spy->tex_path  = "assets/img/buttons/red.png";
        cfg_btn_red_spy->callback  = lobby_button_click;
        btn_red_spy = button_create(context->renderer, 0, cfg_btn_red_spy);
        free(cfg_btn_red_spy);
    } else loading_fails++;

    ButtonConfig* cfg_btn_blue_agent = button_config_init();
    if (cfg_btn_blue_agent) {
        cfg_btn_blue_agent->x         = 400;
        cfg_btn_blue_agent->y         = 260;
        cfg_btn_blue_agent->h         = height;
        cfg_btn_blue_agent->font_path = FONT_LARABIE;
        cfg_btn_blue_agent->color     = COL_WHITE;
        cfg_btn_blue_agent->text      = "Agent bleu";
        cfg_btn_blue_agent->tex_path  = "assets/img/buttons/blue.png";
        cfg_btn_blue_agent->callback  = lobby_button_click;
        btn_blue_agent = button_create(context->renderer, 0, cfg_btn_blue_agent);
        free(cfg_btn_blue_agent);
    } else loading_fails++;

    ButtonConfig* cfg_btn_blue_spy = button_config_init();
    if (cfg_btn_blue_spy) {
        cfg_btn_blue_spy->x         = 400;
        cfg_btn_blue_spy->y         = 0;
        cfg_btn_blue_spy->h         = height;
        cfg_btn_blue_spy->font_path = FONT_LARABIE;
        cfg_btn_blue_spy->color     = COL_WHITE;
        cfg_btn_blue_spy->text      = "Espion bleu";
        cfg_btn_blue_spy->tex_path  = "assets/img/buttons/blue.png";
        cfg_btn_blue_spy->callback  = lobby_button_click;
        btn_blue_spy = button_create(context->renderer, 0, cfg_btn_blue_spy);
        free(cfg_btn_blue_spy);
    } else loading_fails++;

    ButtonConfig* cfg_btn_launch_game = button_config_init();
    if (cfg_btn_launch_game) {
        cfg_btn_launch_game->x         = 0;
        cfg_btn_launch_game->y         = -350;
        cfg_btn_launch_game->h         = height;
        cfg_btn_launch_game->font_path = FONT_LARABIE;
        cfg_btn_launch_game->color     = COL_WHITE;
        cfg_btn_launch_game->text      = "Lancer la partie";
        cfg_btn_launch_game->callback  = lobby_button_click;
        btn_launch_game = button_create(context->renderer, 0, cfg_btn_launch_game);
        free(cfg_btn_launch_game);
    } else loading_fails++;

    ButtonConfig* cfg_btn_return = button_config_init();
    if (cfg_btn_return) {
        cfg_btn_return->x         = -500;
        cfg_btn_return->y         = -450;
        cfg_btn_return->h         = height;
        cfg_btn_return->font_path = FONT_LARABIE;
        cfg_btn_return->color     = COL_WHITE;
        cfg_btn_return->text      = "Retour au menu";
        cfg_btn_return->callback  = lobby_button_click;
        btn_return = button_create(context->renderer, 0, cfg_btn_return);
        free(cfg_btn_return);
    } else loading_fails++;

    WindowConfig* cfg_role_none_window = window_config_init();
    if (cfg_role_none_window) {
        cfg_role_none_window->x = 0;
        cfg_role_none_window->y = 425;
        cfg_role_none_window->w = 450;
        cfg_role_none_window->h = 200;
        cfg_role_none_window->title = "Joueur(s) sans rôle :";
        cfg_role_none_window->bg_color = (SDL_Color){20, 20, 20, 220};
        cfg_role_none_window->titlebar_color = COL_GRAY;
        role_none_window = window_create(0, cfg_role_none_window);
        if (!role_none_window) loading_fails++;
        free(cfg_role_none_window);
    } else loading_fails++;

    WindowConfig* cfg_role_red_agent_window = window_config_init();
    if (cfg_role_red_agent_window) {
        cfg_role_red_agent_window->x = -450;
        cfg_role_red_agent_window->y = 250;
        cfg_role_red_agent_window->w = 300;
        cfg_role_red_agent_window->h = 250;
        cfg_role_red_agent_window->title = "";
        cfg_role_red_agent_window->bg_color = (SDL_Color){150, 50, 50, 220};
        cfg_role_red_agent_window->titlebar_h = 0;
        role_red_agent_window = window_create(0, cfg_role_red_agent_window);
        if (!role_red_agent_window) loading_fails++;
        free(cfg_role_red_agent_window);
    } else loading_fails++;

    WindowConfig* cfg_role_red_spy_window = window_config_init();
    if (cfg_role_red_spy_window) {
        cfg_role_red_spy_window->x = -450;
        cfg_role_red_spy_window->y = -75;
        cfg_role_red_spy_window->w = 300;
        cfg_role_red_spy_window->h = 250;
        cfg_role_red_spy_window->title = "";
        cfg_role_red_spy_window->bg_color = (SDL_Color){150, 50, 50, 220};
        cfg_role_red_spy_window->titlebar_h = 0;
        role_red_spy_window = window_create(0, cfg_role_red_spy_window);
        if (!role_red_spy_window) loading_fails++;
        free(cfg_role_red_spy_window);
    } else loading_fails++;

    WindowConfig* cfg_role_blue_agent_window = window_config_init();
    if (cfg_role_blue_agent_window) {
        cfg_role_blue_agent_window->x = 450;
        cfg_role_blue_agent_window->y = 250;
        cfg_role_blue_agent_window->w = 300;
        cfg_role_blue_agent_window->h = 250;
        cfg_role_blue_agent_window->title = "";
        cfg_role_blue_agent_window->bg_color = (SDL_Color){50, 50, 150, 220};
        cfg_role_blue_agent_window->titlebar_h = 0;
        role_blue_agent_window = window_create(0, cfg_role_blue_agent_window);
        if (!role_blue_agent_window) loading_fails++;
        free(cfg_role_blue_agent_window);
    } else loading_fails++;

    WindowConfig* cfg_role_blue_spy_window = window_config_init();
    if (cfg_role_blue_spy_window) {
        cfg_role_blue_spy_window->x = 450;
        cfg_role_blue_spy_window->y = -75;
        cfg_role_blue_spy_window->w = 300;
        cfg_role_blue_spy_window->h = 250;
        cfg_role_blue_spy_window->title = "";
        cfg_role_blue_spy_window->bg_color = (SDL_Color){50, 50, 150, 220};
        cfg_role_blue_spy_window->titlebar_h = 0;
        role_blue_spy_window = window_create(0, cfg_role_blue_spy_window);
        if (!role_blue_spy_window) loading_fails++;
        free(cfg_role_blue_spy_window);
    } else loading_fails++;


    WindowConfig* cfg_game_options_window = window_config_init();
    if (cfg_game_options_window) {
        cfg_game_options_window->x = 0;
        cfg_game_options_window->y = 50;
        cfg_game_options_window->w = 400;
        cfg_game_options_window->h = 300;
        cfg_game_options_window->title = "Options de la partie";
        cfg_game_options_window->bg_color = (SDL_Color){20, 20, 20, 220};
        game_options_window = window_create(0, cfg_game_options_window);
        if (!game_options_window) loading_fails++;
        free(cfg_game_options_window);
    } else loading_fails++;

    /* Initialisation des textes optimisés */
    txt_lobby_info = init_text(context, "Lobby",
        create_text_config(FONT_LARABIE, 36, COL_WHITE, 0, -250, 0, 255));
    
    txt_no_role_label = init_text(context, "",
        create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 500, 0, 255));
    
    /* Pré-allouer les textes pour les noms de joueurs */
    for (int i = 0; i < MAX_PLAYER_TEXTS; i++) {
        txt_player_names[i] = init_text(context, "",
            create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 0, 0, 255));
    }

    if (loading_fails == 0) {
        printf("Lobby assets loaded successfully\n");
        return EXIT_SUCCESS;
    } else {
        printf("Lobby asset loading failed: %d failures\n", loading_fails);
        return EXIT_FAILURE;
    }
    
}

int struct_lobby_init(Lobby* lobby, int id, const char* code) {
    if (!lobby) return EXIT_FAILURE;

    chat_clear(&lobby->chat);

    for(int i = 0; i < MAX_USERS; i++) {
        if (lobby->users[i] && lobby->users[i]->name) {
            free(lobby->users[i]->name);
            lobby->users[i]->name = NULL;
            free(lobby->users[i]);
            lobby->users[i] = NULL;
        }
    }

    lobby->id = id;
    lobby->status = LB_STATUS_WAITING;
    lobby->nb_players = 0;
    lobby->owner_id = -1;
    lobby->game = NULL;
    strcpy(lobby->code, code);

    for (int i = 0; i < MAX_USERS; i++) {
        lobby->users[i] = NULL;
    }

    if (chat_init(&lobby->chat, CHAT_MAX_MESSAGES) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void lobby_display(AppContext* context) {

    if (!audio_is_playing(MUSIC_MENU_LOBBY)) {
        audio_play_with_fade(MUSIC_MENU_LOBBY, -1, 1500, AUDIO_FADE_IN_BY_VOLUME, NULL);
    }

    /* Réinitialiser le compteur d'index de texte joueur */
    current_player_text_index = 0;

    /* Appliquer le filtre lobby uniquement si nécessaire, selon l'état audio réel. */
    AudioFilterType current_filter = AUDIO_FILTER_NONE;
    float current_cutoff = 0.0f;
    if (audio_get_filter(MUSIC_MENU_LOBBY, &current_filter, &current_cutoff) == EXIT_SUCCESS) {
        if (current_filter != AUDIO_FILTER_LOW_PASS || current_cutoff != 1600.0f) {
            audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_LOW_PASS, 1600.0f);
        }
    }

    /* Afficher le code et l'id du lobby */
    char buf[128];
    format_to(buf, sizeof(buf), "Lobby %d  •  Code : %s",
              context->lobby->id,
              context->lobby->code ? context->lobby->code : "----");

    /* Affichage centré horizontalement, position verticale relative */
    update_text(context, txt_lobby_info, buf);
    update_text_position(txt_lobby_info, 0, -250);
    display_text(context, txt_lobby_info);

    // Affichage des joueurs sans rôle
    display_text(context, txt_no_role_label);

    if (btn_red_agent) button_render(context->renderer, btn_red_agent);
    if (btn_red_spy) button_render(context->renderer, btn_red_spy);
    if (btn_blue_agent) button_render(context->renderer, btn_blue_agent);
    if (btn_blue_spy) button_render(context->renderer, btn_blue_spy);
    if (btn_launch_game) button_render(context->renderer, btn_launch_game);
    if (btn_return) button_render(context->renderer, btn_return);

    if (role_none_window) window_render(context->renderer, role_none_window);
    if (role_red_agent_window) {
        window_render(context->renderer, role_red_agent_window);
        if (btn_red_agent) {
            window_place_button(role_red_agent_window, btn_red_agent, 0, 80);
            button_render(context->renderer, btn_red_agent);
        }
    }
    if (role_red_spy_window) {
        window_render(context->renderer, role_red_spy_window);
        if (btn_red_spy) {
            window_place_button(role_red_spy_window, btn_red_spy, 0, 80);
            button_render(context->renderer, btn_red_spy);
        }
    }
    if (role_blue_agent_window) {
        window_render(context->renderer, role_blue_agent_window);
        if (btn_blue_agent) {
            window_place_button(role_blue_agent_window, btn_blue_agent, 0, 80);
            button_render(context->renderer, btn_blue_agent);
        }
    }
    if (role_blue_spy_window) {
        window_render(context->renderer, role_blue_spy_window);
        if (btn_blue_spy) {
            window_place_button(role_blue_spy_window, btn_blue_spy, 0, 80);
            button_render(context->renderer, btn_blue_spy);
        }
    }

    if (game_options_window) {
        window_render(context->renderer, game_options_window);
    }

    // Comptage des joueurs par rôle/équipe et indices d'affichage
    PlayerPlacementCounters counters = {0};

    // Comptage du joueur local
    User user = {-1, context->player_name, context->player_role, context->player_team};

    count_player_for_layout(&user, &counters);

    // Comptage des joueurs distants
    for (int i = 0; i < MAX_USERS; i++) {
        User* remote_user = context->lobby->users[i];
        if (!remote_user) continue;
        count_player_for_layout(remote_user, &counters);
    }

    // Affichage du joueur local
    player_display(context, &user,
        counters.nb_none, counters.i_none,
        counters.nb_red_spy, counters.i_red_spy,
        counters.nb_red_agent, counters.i_red_agent,
        counters.nb_blue_spy, counters.i_blue_spy,
        counters.nb_blue_agent, counters.i_blue_agent);
    advance_player_layout_index(&user, &counters);

    // Affichage des joueurs distants
    for (int i = 0; i < MAX_USERS; i++) {
        User* remote_user = context->lobby->users[i];
        if (!remote_user) continue;
        player_display(context, remote_user,
            counters.nb_none, counters.i_none,
            counters.nb_red_spy, counters.i_red_spy,
            counters.nb_red_agent, counters.i_red_agent,
            counters.nb_blue_spy, counters.i_blue_spy,
            counters.nb_blue_agent, counters.i_blue_agent);
        advance_player_layout_index(remote_user, &counters);
    }

}

void lobby_handle_event(AppContext* context, SDL_Event* e) {
    if (!context || !e) return;

    if (btn_red_agent) button_handle_event(context, btn_red_agent, e);
    if (btn_red_spy) button_handle_event(context, btn_red_spy, e);
    if (btn_blue_agent) button_handle_event(context, btn_blue_agent, e);
    if (btn_blue_spy) button_handle_event(context, btn_blue_spy, e);
    if (btn_launch_game) button_handle_event(context, btn_launch_game, e);
    if (btn_return) button_handle_event(context, btn_return, e);

    if (role_none_window) window_handle_event(context, role_none_window, e);
    if (role_red_agent_window) window_handle_event(context, role_red_agent_window, e);
    if (role_red_spy_window) window_handle_event(context, role_red_spy_window, e);
    if (role_blue_agent_window) window_handle_event(context, role_blue_agent_window, e);
    if (role_blue_spy_window) window_handle_event(context, role_blue_spy_window, e);

    if (game_options_window) window_handle_event(context, game_options_window, e);
}

void player_display(AppContext* context, User* user, int nb_none, int i_none, int nb_red_spy, int i_red_spy, int nb_red_agent, int i_red_agent, int nb_blue_spy, int i_blue_spy, int nb_blue_agent, int i_blue_agent) {
    if (!context || !user) return;
    if (!user->name) return;

    SDL_Texture* icon = NULL;
    Window* target_window = NULL;
    int base_rel_x = 0;
    int base_rel_y = 0;
    int nb_player = 0;
    int i_player = 0;

    if (user->team == TEAM_RED) {
        icon = player_icon_red;
        if (user->role == ROLE_AGENT) {
            /* Conserve la disposition actuelle: agents rouges dans la fenêtre haute. */
            target_window = role_red_agent_window;
            base_rel_y = 10;
            nb_player = nb_red_agent;
            i_player = i_red_agent;
        } else if (user->role == ROLE_SPY) {
            /* Conserve la disposition actuelle: espions rouges dans la fenêtre basse. */
            target_window = role_red_spy_window;
            base_rel_y = 10;
            nb_player = nb_red_spy;
            i_player = i_red_spy;
        } else {
            // Rôle non assigné mais équipe rouge
            target_window = role_none_window;
            base_rel_y = 15;
            nb_player = nb_none;
            i_player = i_none;
        }
    } else if (user->team == TEAM_BLUE) {
        icon = player_icon_blue;
        if (user->role == ROLE_AGENT) {
            /* Conserve la disposition actuelle: agents bleus dans la fenêtre haute. */
            target_window = role_blue_agent_window;
            base_rel_y = 10;
            nb_player = nb_blue_agent;
            i_player = i_blue_agent;
        } else if (user->role == ROLE_SPY) {
            /* Conserve la disposition actuelle: espions bleus dans la fenêtre basse. */
            target_window = role_blue_spy_window;
            base_rel_y = 10;
            nb_player = nb_blue_spy;
            i_player = i_blue_spy;
        } else {
            target_window = role_none_window;
            base_rel_y = 15;
            nb_player = nb_none;
            i_player = i_none;
        }
    } else {
        // Équipe non assignée (TEAM_NONE) - afficher au centre
        icon = player_icon_none;
        target_window = role_none_window;
        base_rel_y = 15;
        nb_player = nb_none;
        i_player = i_none;
    }

    player_icon_pos(context, user, icon, target_window, nb_player, i_player, base_rel_x, base_rel_y);
}

void player_icon_pos(AppContext* context, User* user, SDL_Texture* icon, Window* target_window, int nb_player, int i_player, int base_rel_x, int base_rel_y) {
    if (!context){
        printf("Invalid context\n");
        return;
    }
    if (!user) {
        printf("Invalid user\n");
        return;
    }
    if (!icon) {
        printf("Invalid icon\n");
        return;
    }
    if (!target_window) {
        printf("Invalid target window\n");
        return;
    }

    int x = 0;
    int y = 0;
    if (!compute_player_icon_position(nb_player, i_player, base_rel_x, base_rel_y, &x, &y)) {
        return;
    }

    window_display_image(context->renderer, target_window, icon, x, y, 0.25f, 0, SDL_FLIP_NONE, 1, 255);

    if (current_player_text_index >= MAX_PLAYER_TEXTS) return;
    if (!user->name) return;

    Text* txt = txt_player_names[current_player_text_index];
    if (txt) {
        update_text(context, txt, user->name);
        window_place_text(target_window, txt, x, y - 25);
        display_text(context, txt);
    }
    current_player_text_index++;
}

int lobby_free(){
    /* détruire textures d'assets chargées par lobby */

    if (btn_red_agent)  { button_destroy(btn_red_agent);  btn_red_agent = NULL; }
    if (btn_red_spy)    { button_destroy(btn_red_spy);    btn_red_spy = NULL; }
    if (btn_blue_agent) { button_destroy(btn_blue_agent); btn_blue_agent = NULL; }
    if (btn_blue_spy)   { button_destroy(btn_blue_spy);   btn_blue_spy = NULL; }
    if (btn_launch_game) { button_destroy(btn_launch_game); btn_launch_game = NULL; }
    if (btn_return)     { button_destroy(btn_return);     btn_return = NULL; }

    if (role_none_window) { window_destroy(role_none_window); role_none_window = NULL; }
    if (role_red_agent_window) { window_destroy(role_red_agent_window); role_red_agent_window = NULL; }
    if (role_red_spy_window) { window_destroy(role_red_spy_window); role_red_spy_window = NULL; }
    if (role_blue_agent_window) { window_destroy(role_blue_agent_window); role_blue_agent_window = NULL; }
    if (role_blue_spy_window) { window_destroy(role_blue_spy_window); role_blue_spy_window = NULL; }

    if (game_options_window) { window_destroy(game_options_window); game_options_window = NULL; }

    if (player_icon_none) { free_image(player_icon_none); player_icon_none = NULL; }
    if (player_icon_red)  { free_image(player_icon_red);  player_icon_red = NULL; }
    if (player_icon_blue) { free_image(player_icon_blue); player_icon_blue = NULL; }

    /* Libération des textes optimisés */
    destroy_text(txt_lobby_info); txt_lobby_info = NULL;
    destroy_text(txt_no_role_label); txt_no_role_label = NULL;
    
    for (int i = 0; i < MAX_PLAYER_TEXTS; i++) {
        destroy_text(txt_player_names[i]);
        txt_player_names[i] = NULL;
    }

    return EXIT_SUCCESS;
}