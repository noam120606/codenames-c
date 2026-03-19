#include "../lib/all.h"

static int lobby_filter_applied = 0;
Button* btn_red_agent = NULL;
Button* btn_red_spy = NULL;
Button* btn_blue_agent = NULL;
Button* btn_blue_spy = NULL;
Button* btn_launch_game = NULL;
Button* btn_return = NULL;

/* Callback pour boutons du lobby (choix de rôle / équipe) */
static ButtonReturn lobby_button_click(SDL_Context* context, Button* button) {
    if (!context || !button) return BTN_RET_NONE;
    if (button == btn_red_agent) {
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_RED;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_AGENT, TEAM_RED);
        send_tcp(context->sock, msg);
        printf("Selected role: RED AGENT\n");
    } else if (button == btn_red_spy) {
        context->player_role = ROLE_SPY;
        context->player_team = TEAM_RED;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_SPY, TEAM_RED);
        send_tcp(context->sock, msg);
        printf("Selected role: RED SPY\n");
    } else if (button == btn_blue_agent) {
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_BLUE;
        char msg[16];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_CHOOSE_ROLE, ROLE_AGENT, TEAM_BLUE);
        send_tcp(context->sock, msg);
        printf("Selected role: BLUE AGENT\n");
    } else if (button == btn_blue_spy) {
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
            char msg[16];
            format_to(msg, sizeof(msg), "%d", MSG_LEAVELOBBY);
            send_tcp(context->sock, msg);
        }
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;
        context->lobby->id = -1;

        /* Retirer le filtre audio en quittant le lobby */
        audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_NONE, 0);
        lobby_filter_applied = 0;
        context->app_state = APP_STATE_MENU;
        printf("Returned to menu\n");
    }
    return BTN_RET_NONE;
}

int lobby_init(SDL_Context* context) {

    int loading_fails = 0;

    /* Créer les boutons de rôle/équipe via ButtonConfig */
    int height   = 64;

    ButtonConfig* cfg_btn_red_agent = button_config_init();
    if (cfg_btn_red_agent) {
        cfg_btn_red_agent->x         = -360;
        cfg_btn_red_agent->y         = 120;
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
        cfg_btn_red_spy->x         = -360;
        cfg_btn_red_spy->y         = -120;
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
        cfg_btn_blue_agent->x         = 360;
        cfg_btn_blue_agent->y         = 120;
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
        cfg_btn_blue_spy->x         = 360;
        cfg_btn_blue_spy->y         = -120;
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


    if (loading_fails == 0) {
        printf("Lobby assets loaded successfully\n");
        return EXIT_SUCCESS;
    } else {
        printf("Lobby asset loading failed: %d failures\n", loading_fails);
        return EXIT_FAILURE;
    }
    
}

void lobby_display(SDL_Context* context) {

    /* Application d'un filtre sur la musique du menu (une seule fois) */
    if (!lobby_filter_applied) {
        audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_LOW_PASS, 2200);
        lobby_filter_applied = 1;
    }

    /* Afficher le code et l'id du lobby */
    char buf[128];
    format_to(buf, sizeof(buf), "Lobby %d  •  Code : %s",
              context->lobby->id,
              context->lobby->code ? context->lobby->code : "----");

    /* Affichage centré horizontalement, position verticale relative */
    int rel_x = 0;
    int rel_y = -200;
    text_display(context->renderer, buf, FONT_LARABIE, 36, COL_WHITE, rel_x, rel_y, 0, 255);

    /* afficher la liste des joueurs ici (nécessite que le client reçoive la liste du serveur). */

    button_render(context->renderer, btn_red_agent);
    button_render(context->renderer, btn_red_spy);
    button_render(context->renderer, btn_blue_agent);
    button_render(context->renderer, btn_blue_spy);
    button_render(context->renderer, btn_launch_game);
    button_render(context->renderer, btn_return);
}

ButtonReturn lobby_handle_event(SDL_Context* context, SDL_Event* e) {
    if (!context || !e) return BTN_RET_NONE;

    ButtonReturn ret = BTN_RET_NONE;
    ButtonReturn r;
    if (btn_red_agent)  { r = button_handle_event(context, btn_red_agent, e);  if (r != BTN_RET_NONE) ret = r; }
    if (btn_red_spy)    { r = button_handle_event(context, btn_red_spy, e);    if (r != BTN_RET_NONE) ret = r; }
    if (btn_blue_agent) { r = button_handle_event(context, btn_blue_agent, e); if (r != BTN_RET_NONE) ret = r; }
    if (btn_blue_spy)   { r = button_handle_event(context, btn_blue_spy, e);   if (r != BTN_RET_NONE) ret = r; }
    if (btn_return)     { r = button_handle_event(context, btn_return, e);     if (r != BTN_RET_NONE) ret = r; }
    return ret;
}

int lobby_free(){
    /* détruire textures d'assets chargées par lobby */

    if (btn_red_agent)  { button_destroy(btn_red_agent);  btn_red_agent = NULL; }
    if (btn_red_spy)    { button_destroy(btn_red_spy);    btn_red_spy = NULL; }
    if (btn_blue_agent) { button_destroy(btn_blue_agent); btn_blue_agent = NULL; }
    if (btn_blue_spy)   { button_destroy(btn_blue_spy);   btn_blue_spy = NULL; }
    if (btn_launch_game) { button_destroy(btn_launch_game); btn_launch_game = NULL; }
    if (btn_return)     { button_destroy(btn_return);     btn_return = NULL; }

    return EXIT_SUCCESS;
}