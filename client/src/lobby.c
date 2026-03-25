#include "../lib/all.h"

Button* btn_red_agent = NULL;
Button* btn_red_spy = NULL;
Button* btn_blue_agent = NULL;
Button* btn_blue_spy = NULL;
Button* btn_launch_game = NULL;
Button* btn_return = NULL;
SDL_Texture* player_icon_none = NULL;
SDL_Texture* player_icon_red = NULL;
SDL_Texture* player_icon_blue = NULL;

/* Textes optimisés pour le lobby */
static Text* txt_lobby_info = NULL;
static Text* txt_no_role_label = NULL;
#define MAX_PLAYER_TEXTS 16
static Text* txt_player_names[MAX_PLAYER_TEXTS] = {NULL};
static int current_player_text_index = 0;

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

    /* Initialisation des textes optimisés */
    txt_lobby_info = init_text(context, "Lobby",
        create_text_config(FONT_LARABIE, 36, COL_WHITE, 0, -250, 0, 255));
    
    txt_no_role_label = init_text(context, "Joueur(s) sans rôle :",
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

    /* afficher la liste des joueurs ici (nécessite que le client reçoive la liste du serveur). */

    button_render(context->renderer, btn_red_agent);
    button_render(context->renderer, btn_red_spy);
    button_render(context->renderer, btn_blue_agent);
    button_render(context->renderer, btn_blue_spy);
    button_render(context->renderer, btn_launch_game);
    button_render(context->renderer, btn_return);

    // Comptage des joueurs par rôle/équipe
    int nb_none = 0;
    int nb_red_spy = 0;
    int nb_red_agent = 0;
    int nb_blue_spy = 0;
    int nb_blue_agent = 0;

    // Indices des joueurs
    int i_none = 0;
    int i_red_spy = 0;
    int i_red_agent = 0;
    int i_blue_spy = 0;
    int i_blue_agent = 0;

    // Comptage du joueur local
    User user = {-1, context->player_name, context->player_role, context->player_team};

    switch(user.team) {
        case TEAM_RED: user.role == ROLE_SPY ? nb_red_spy++ : nb_red_agent++; break;
        case TEAM_BLUE: user.role == ROLE_SPY ? nb_blue_spy++ : nb_blue_agent++; break;
        case TEAM_NONE: nb_none++; break;
        default: printf("Warning: User %s has invalid team %d\n", user.name, user.team); break;
    }

    // Comptage des joueurs distants
    for (int i = 0; i < MAX_USERS; i++) {
        User* user = context->lobby->users[i];
        if (!user) continue;
        switch(user->team) {
            case TEAM_RED: user->role == ROLE_SPY ? nb_red_spy++ : nb_red_agent++; break;
            case TEAM_BLUE: user->role == ROLE_SPY ? nb_blue_spy++ : nb_blue_agent++; break;
            case TEAM_NONE: nb_none++; break;
            default: printf("Warning: User %s has invalid team %d\n", user->name, user->team); break;
        }
    }

    // Affichage du joueur local
    player_display(context, &user, nb_none, 0, nb_red_spy, 0, nb_red_agent, 0, nb_blue_spy, 0, nb_blue_agent, 0);
    switch(user.team) {
        case TEAM_RED: user.role == ROLE_SPY ? i_red_spy++ : i_red_agent++; break;
        case TEAM_BLUE: user.role == ROLE_SPY ? i_blue_spy++ : i_blue_agent++; break;
        case TEAM_NONE: i_none++; break;
        default: printf("Warning: User %s has invalid team %d\n", user.name, user.team); break;
    }

    // Affichage des joueurs distants
    for (int i = 0; i < MAX_USERS; i++) {
        User* user = context->lobby->users[i];
        if (!user) continue;
        player_display(context, user, nb_none, i_none, nb_red_spy, i_red_spy, nb_red_agent, i_red_agent, nb_blue_spy, i_blue_spy, nb_blue_agent, i_blue_agent);
        switch(user->team) {
            case TEAM_RED: user->role == ROLE_SPY ? i_red_spy++ : i_red_agent++; break;
            case TEAM_BLUE: user->role == ROLE_SPY ? i_blue_spy++ : i_blue_agent++; break;
            case TEAM_NONE: i_none++; break;
            default: printf("Warning: User %s has invalid team %d\n", user->name, user->team); break;
        }
    }

}

ButtonReturn lobby_handle_event(AppContext* context, SDL_Event* e) {
    if (!context || !e) return BTN_RET_NONE;

    ButtonReturn ret = BTN_RET_NONE;
    ButtonReturn r;
    if (btn_red_agent)  { r = button_handle_event(context, btn_red_agent, e);  if (r != BTN_RET_NONE) ret = r; }
    if (btn_red_spy)    { r = button_handle_event(context, btn_red_spy, e);    if (r != BTN_RET_NONE) ret = r; }
    if (btn_blue_agent) { r = button_handle_event(context, btn_blue_agent, e); if (r != BTN_RET_NONE) ret = r; }
    if (btn_blue_spy)   { r = button_handle_event(context, btn_blue_spy, e);   if (r != BTN_RET_NONE) ret = r; }
    if (btn_launch_game) { r = button_handle_event(context, btn_launch_game, e); if (r != BTN_RET_NONE) ret = r; }
    if (btn_return)     { r = button_handle_event(context, btn_return, e);     if (r != BTN_RET_NONE) ret = r; }
    return ret;
}

void player_display(AppContext* context, User* user, int nb_none, int i_none, int nb_red_spy, int i_red_spy, int nb_red_agent, int i_red_agent, int nb_blue_spy, int i_blue_spy, int nb_blue_agent, int i_blue_agent) {
    if (!context || !user) return;
    if (!user->name) return;

    SDL_Texture* icon = NULL;
    int base_x = 0;
    int base_y = 0;
    int index = 0;

    // Positions des boutons (mêmes que dans lobby_init)
    const int center_none_x = 0;
    const int center_none_y = 500;
    const int center_red_x = -400;
    const int center_blue_x = 400;
    const int center_agent_y = 260;
    const int center_spy_y = 0;

    // Décalages
    const int offset_y = -80;

    if (user->team == TEAM_RED) {
        icon = player_icon_red;
        base_x = center_red_x;
        if (user->role == ROLE_SPY) {
            base_y = center_spy_y + offset_y;
            index = nb_red_spy;
            player_icon_pos(context, user, icon, nb_red_spy, i_red_spy, base_x, base_y);
        } else if (user->role == ROLE_AGENT) {
            base_y = center_agent_y + offset_y;
            index = nb_red_agent;
            player_icon_pos(context, user, icon, nb_red_agent, i_red_agent, base_x, base_y);
        } else {
            // Rôle non assigné mais équipe rouge
            base_y = center_none_x;
            index = nb_none;
            /* draw unassigned red-team player using the same positioning routine */
            player_icon_pos(context, user, icon, nb_none, i_none, base_x, base_y);
        }
    } else if (user->team == TEAM_BLUE) {
        icon = player_icon_blue;
        base_x = center_blue_x;
        if (user->role == ROLE_SPY) {
            base_y = center_spy_y + offset_y;
            index = nb_blue_spy;
            player_icon_pos(context, user, icon, nb_blue_spy, i_blue_spy, base_x, base_y);
        } else if (user->role == ROLE_AGENT) {
            base_y = center_agent_y + offset_y;
            index = nb_blue_agent;
            player_icon_pos(context, user, icon, nb_blue_agent, i_blue_agent, base_x, base_y);
        } else {
            // Rôle non assigné mais équipe bleue
            base_y = center_none_x;
            index = i_none;
            /* draw unassigned blue-team player */
            player_icon_pos(context, user, icon, nb_none, i_none, base_x, base_y);
        }
    } else {
        // Équipe non assignée (TEAM_NONE) - afficher au centre
        icon = player_icon_none;
        base_x = center_none_x;
        base_y = center_none_y + offset_y;
        index = nb_none;
    player_icon_pos(context, user, icon, nb_none, i_none, base_x, base_y);
    }
}

/* Affiche le nom d'un joueur en utilisant les textes pré-alloués */
static void display_player_name(AppContext* context, const char* name, int x, int y) {
    if (current_player_text_index >= MAX_PLAYER_TEXTS) return;
    if (!name) return;
    
    Text* txt = txt_player_names[current_player_text_index];
    if (txt) {
        update_text(context, txt, name);
        update_text_position(txt, x, y);
        display_text(context, txt);
    }
    current_player_text_index++;
}

void player_icon_pos(AppContext* context, User* user, SDL_Texture* icon, int nb_player, int i_player, int base_x, int base_y) {
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

    // Décalage horizontal entre chaque joueur du même rôle
    const int spacing_x = 80;
    const int spacing_y = 80;

    if(nb_player > 0 && nb_player <= 4){ // Les joueurs sont affichés sur 1 étage (à la suite)
        int x = base_x -spacing_x * 2 + spacing_x / 2 + i_player * spacing_x;
        int y = base_y;
        // Afficher l'icône puis du pseudo
        display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
        display_player_name(context, user->name, x, y - 25);
    } else if(nb_player > 4 && nb_player <= 8){ // Les joueurs sont affichés sur 2 étages
        if(nb_player % 2){
            if(i_player <= 4){
                int x = base_x -spacing_x * 2 + spacing_x / 2 + i_player * spacing_x;
                int y = base_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            } else {
                int x = base_x -spacing_x * 2 + spacing_x / 2 + (i_player - 4) * spacing_x;
                int y = base_y - spacing_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            }
        } else if(nb_player == 5){
            if(i_player < 3){ // On affiche 3 joueurs au premier étage
                int x = base_x -spacing_x * 1.5 + i_player * spacing_x;
                int y = base_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            } else { // On affiche 2 joueurs au deuxième étage
                int x = base_x -spacing_x * 2 + spacing_x / 2 + i_player * spacing_x;
                int y = base_y - spacing_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            }
        } else if(nb_player == 7){
            if(i_player < 4){ // On affiche 4 joueurs au premier étage
                int x = base_x -spacing_x * 2 + spacing_x / 2 + i_player * spacing_x;
                int y = base_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            } else { // On affiche 3 joueurs au deuxième étage
                int x = base_x -spacing_x * 1.5 + (i_player - 4) * spacing_x;
                int y = base_y - spacing_y;
                display_image(context->renderer, icon, x, y, 0.25, 0, SDL_FLIP_NONE, 1, 255);
                display_player_name(context, user->name, x, y - 25);
            }
        }
    } else {
        printf("Too many players for lobby display: %d\n", nb_player);
        return;
    }
}

int lobby_free(){
    /* détruire textures d'assets chargées par lobby */

    if (btn_red_agent)  { button_destroy(btn_red_agent);  btn_red_agent = NULL; }
    if (btn_red_spy)    { button_destroy(btn_red_spy);    btn_red_spy = NULL; }
    if (btn_blue_agent) { button_destroy(btn_blue_agent); btn_blue_agent = NULL; }
    if (btn_blue_spy)   { button_destroy(btn_blue_spy);   btn_blue_spy = NULL; }
    if (btn_launch_game) { button_destroy(btn_launch_game); btn_launch_game = NULL; }
    if (btn_return)     { button_destroy(btn_return);     btn_return = NULL; }

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