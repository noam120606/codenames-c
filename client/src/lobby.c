#include "../lib/all.h"

static SDL_Texture* lobby_btn_red = NULL;
static SDL_Texture* lobby_btn_blue = NULL;
Button* btn_red_agent = NULL;
Button* btn_red_spy = NULL;
Button* btn_blue_agent = NULL;
Button* btn_blue_spy = NULL;
Button* btn_return = NULL;

/* Callback pour boutons du lobby (choix de rôle / équipe) */
static ButtonReturn lobby_button_click(SDL_Context* context, Button* button) {
    if (!context || !button) return BTN_RET_NONE;
    if (button == btn_red_agent) {
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_RED;
        printf("Selected role: RED AGENT\n");
    } else if (button == btn_red_spy) {
        context->player_role = ROLE_SPY;
        context->player_team = TEAM_RED;
        printf("Selected role: RED SPY\n");
    } else if (button == btn_blue_agent) {
        context->player_role = ROLE_AGENT;
        context->player_team = TEAM_BLUE;
        printf("Selected role: BLUE AGENT\n");
    } else if (button == btn_blue_spy) {
        context->player_role = ROLE_SPY;
        context->player_team = TEAM_BLUE;
        printf("Selected role: BLUE SPY\n");
    } else if (button == btn_return) {
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;
        context->lobby_id = -1;
        if (context->lobby_code) {
            free(context->lobby_code);
            context->lobby_code = NULL;
        }
        context->game_state = GAME_STATE_MENU;
        printf("Returned to menu\n");
    }
    /* Optionnel : informer le serveur du choix (si protocole / message existant)
       char msg[64];
       format_to(msg, sizeof(msg), "MSG_SETROLE %d %d", context->player_role, context->player_team);
       send_tcp(context->sock, msg);
    */
    return BTN_RET_NONE;
}

int lobby_init(SDL_Context* context) {

    int load_errors = 0;

    lobby_btn_red = IMG_LoadTexture(context->renderer, "assets/img/buttons/red.png");
    if (!lobby_btn_red) {
        fprintf(stderr, "lobby_init: failed to load assets/img/buttons/red.png: %s\n", IMG_GetError());
        load_errors++;
    }

    lobby_btn_blue = IMG_LoadTexture(context->renderer, "assets/img/buttons/blue.png");
    if (!lobby_btn_blue) {
        fprintf(stderr, "lobby_init: failed to load assets/img/buttons/blue.png: %s\n", IMG_GetError());
        load_errors++;
    }

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
        cfg_btn_red_agent->callback  = lobby_button_click;
        btn_red_agent = button_create(context->renderer, 0, cfg_btn_red_agent);
        free(cfg_btn_red_agent);
    }

    ButtonConfig* cfg_btn_red_spy = button_config_init();
    if (cfg_btn_red_spy) {
        cfg_btn_red_spy->x         = -360;
        cfg_btn_red_spy->y         = -120;
        cfg_btn_red_spy->h         = height;
        cfg_btn_red_spy->font_path = FONT_LARABIE;
        cfg_btn_red_spy->color     = COL_WHITE;
        cfg_btn_red_spy->text      = "Espion rouge";
        cfg_btn_red_spy->callback  = lobby_button_click;
        btn_red_spy = button_create(context->renderer, 0, cfg_btn_red_spy);
        free(cfg_btn_red_spy);
    }

    ButtonConfig* cfg_btn_blue_agent = button_config_init();
    if (cfg_btn_blue_agent) {
        cfg_btn_blue_agent->x         = 360;
        cfg_btn_blue_agent->y         = 120;
        cfg_btn_blue_agent->h         = height;
        cfg_btn_blue_agent->font_path = FONT_LARABIE;
        cfg_btn_blue_agent->color     = COL_WHITE;
        cfg_btn_blue_agent->text      = "Agent bleu";
        cfg_btn_blue_agent->callback  = lobby_button_click;
        btn_blue_agent = button_create(context->renderer, 0, cfg_btn_blue_agent);
        free(cfg_btn_blue_agent);
    }

    ButtonConfig* cfg_btn_blue_spy = button_config_init();
    if (cfg_btn_blue_spy) {
        cfg_btn_blue_spy->x         = 360;
        cfg_btn_blue_spy->y         = -120;
        cfg_btn_blue_spy->h         = height;
        cfg_btn_blue_spy->font_path = FONT_LARABIE;
        cfg_btn_blue_spy->color     = COL_WHITE;
        cfg_btn_blue_spy->text      = "Espion bleu";
        cfg_btn_blue_spy->callback  = lobby_button_click;
        btn_blue_spy = button_create(context->renderer, 0, cfg_btn_blue_spy);
        free(cfg_btn_blue_spy);
    }

    ButtonConfig* cfg_btn_return = button_config_init();
    if (cfg_btn_return) {
        cfg_btn_return->x         = 0;
        cfg_btn_return->y         = -400;
        cfg_btn_return->h         = height;
        cfg_btn_return->font_path = FONT_LARABIE;
        cfg_btn_return->color     = COL_WHITE;
        cfg_btn_return->text      = "Retour au menu";
        cfg_btn_return->callback  = lobby_button_click;
        btn_return = button_create(context->renderer, 0, cfg_btn_return);
        free(cfg_btn_return);
    }
  
    return load_errors;
}

void lobby_display(SDL_Context* context) {

    /* Afficher le code et l'id du lobby */
    char buf[128];
    format_to(buf, sizeof(buf), "Lobby %d  •  Code : %s",
              context->lobby_id,
              context->lobby_code ? context->lobby_code : "----");

    /* Affichage centré horizontalement, position verticale relative */
    int rel_x = 0;
    int rel_y = -200;
    text_display(context->renderer, buf, FONT_LARABIE, 36, COL_WHITE, rel_x, rel_y, 0, 255);

    /* afficher la liste des joueurs ici (nécessite que le client reçoive la liste du serveur). */

    button_render(context->renderer, btn_red_agent);
    button_render(context->renderer, btn_red_spy);
    button_render(context->renderer, btn_blue_agent);
    button_render(context->renderer, btn_blue_spy);
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
    if (btn_return)     { button_destroy(btn_return);     btn_return = NULL; }

    if (lobby_btn_red) {
        SDL_DestroyTexture(lobby_btn_red);
        lobby_btn_red = NULL;
    }
    if (lobby_btn_blue) {
        SDL_DestroyTexture(lobby_btn_blue);
        lobby_btn_blue = NULL;
    }

    return EXIT_SUCCESS;
}