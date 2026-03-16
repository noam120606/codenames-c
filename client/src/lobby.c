#include "../lib/all.h"

static SDL_Texture* lobby_btn_red = NULL;
static SDL_Texture* lobby_btn_blue = NULL;
Button* btn_red_agent = NULL;
Button* btn_red_spy = NULL;
Button* btn_blue_agent = NULL;
Button* btn_blue_spy = NULL;
Button* btn_return = NULL;

/* Callback pour boutons du lobby (choix de rôle / équipe) */
static ButtonReturn lobby_button_click(SDL_Context* context, ButtonId button_id) {
    if (!context) return BTN_RET_NONE;
    switch (button_id) {
        case BTN_RED_AGENT:
            context->player_role = ROLE_AGENT;
            context->player_team = TEAM_RED;
            printf("Selected role: RED AGENT\n");
            break;
        case BTN_RED_SPY:
            context->player_role = ROLE_SPY;
            context->player_team = TEAM_RED;
            printf("Selected role: RED SPY\n");
            break;
        case BTN_BLUE_AGENT:
            context->player_role = ROLE_AGENT;
            context->player_team = TEAM_BLUE;
            printf("Selected role: BLUE AGENT\n");
            break;
        case BTN_BLUE_SPY:
            context->player_role = ROLE_SPY;
            context->player_team = TEAM_BLUE;
            printf("Selected role: BLUE SPY\n");
            break;
        case BTN_RETURN:
            context->player_role = ROLE_NONE;
            context->player_team = TEAM_NONE;
            context->lobby_id = -1;
            if (context->lobby_code) {
                free(context->lobby_code);
                context->lobby_code = NULL;
            }
            context->game_state = GAME_STATE_MENU;
            edit_btn_cfg(BTN_RED_AGENT, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_RED_SPY, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_BLUE_AGENT, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_BLUE_SPY, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_RETURN, BTN_CFG_HIDDEN, 1);
            printf("Returned to menu\n");
            break;
        default:
            break;
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
        cfg_btn_red_agent->hidden    = 1;
        cfg_btn_red_agent->font_path = FONT_LARABIE;
        cfg_btn_red_agent->color     = COL_WHITE;
        cfg_btn_red_agent->text      = "Agent rouge";
        cfg_btn_red_agent->callback  = lobby_button_click;
        btn_red_agent = button_create(context->renderer, BTN_RED_AGENT, cfg_btn_red_agent);
        free(cfg_btn_red_agent);
    }

    ButtonConfig* cfg_btn_red_spy = button_config_init();
    if (cfg_btn_red_spy) {
        cfg_btn_red_spy->x         = -360;
        cfg_btn_red_spy->y         = -120;
        cfg_btn_red_spy->h         = height;
        cfg_btn_red_spy->hidden    = 1;
        cfg_btn_red_spy->font_path = FONT_LARABIE;
        cfg_btn_red_spy->color     = COL_WHITE;
        cfg_btn_red_spy->text      = "Espion rouge";
        cfg_btn_red_spy->callback  = lobby_button_click;
        btn_red_spy = button_create(context->renderer, BTN_RED_SPY, cfg_btn_red_spy);
        free(cfg_btn_red_spy);
    }

    ButtonConfig* cfg_btn_blue_agent = button_config_init();
    if (cfg_btn_blue_agent) {
        cfg_btn_blue_agent->x         = 360;
        cfg_btn_blue_agent->y         = 120;
        cfg_btn_blue_agent->h         = height;
        cfg_btn_blue_agent->hidden    = 1;
        cfg_btn_blue_agent->font_path = FONT_LARABIE;
        cfg_btn_blue_agent->color     = COL_WHITE;
        cfg_btn_blue_agent->text      = "Agent bleu";
        cfg_btn_blue_agent->callback  = lobby_button_click;
        btn_blue_agent = button_create(context->renderer, BTN_BLUE_AGENT, cfg_btn_blue_agent);
        free(cfg_btn_blue_agent);
    }

    ButtonConfig* cfg_btn_blue_spy = button_config_init();
    if (cfg_btn_blue_spy) {
        cfg_btn_blue_spy->x         = 360;
        cfg_btn_blue_spy->y         = -120;
        cfg_btn_blue_spy->h         = height;
        cfg_btn_blue_spy->hidden    = 1;
        cfg_btn_blue_spy->font_path = FONT_LARABIE;
        cfg_btn_blue_spy->color     = COL_WHITE;
        cfg_btn_blue_spy->text      = "Espion bleu";
        cfg_btn_blue_spy->callback  = lobby_button_click;
        btn_blue_spy = button_create(context->renderer, BTN_BLUE_SPY, cfg_btn_blue_spy);
        free(cfg_btn_blue_spy);
    }

    ButtonConfig* cfg_btn_return = button_config_init();
    if (cfg_btn_return) {
        cfg_btn_return->x         = 0;
        cfg_btn_return->y         = -400;
        cfg_btn_return->h         = height;
        cfg_btn_return->hidden    = 1;
        cfg_btn_return->font_path = FONT_LARABIE;
        cfg_btn_return->color     = COL_WHITE;
        cfg_btn_return->text      = "Retour au menu";
        cfg_btn_return->callback  = lobby_button_click;
        btn_return = button_create(context->renderer, BTN_RETURN, cfg_btn_return);
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

    button_render(BTN_RED_AGENT);
    button_render(BTN_RED_SPY);
    button_render(BTN_BLUE_AGENT);
    button_render(BTN_BLUE_SPY);
    button_render(BTN_RETURN);

}

void lobby_handle_event(SDL_Context* context, SDL_Event* e) {
    if (!context || !e) return;
}

int lobby_free(){
    /* détruire textures d'assets chargées par lobby */

    btn_red_agent = NULL;
    btn_red_spy = NULL;
    btn_blue_agent = NULL;
    btn_blue_spy = NULL;
    btn_return = NULL;

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