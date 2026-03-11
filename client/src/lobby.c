#include "../lib/all.h"

static int lobby_buttons_created = 0;

/* Callback pour boutons du lobby (choix de rôle / équipe) */
static ButtonReturn lobby_button_click(SDL_Context* context, ButtonId button_id) {
    if (!context) return BTN_RET_NONE;
    switch (button_id) {
        case BTN_AGENT:
            context->player_role = ROLE_AGENT;
            printf("Selected role: AGENT\n");
            break;
        case BTN_SPY:
            context->player_role = ROLE_SPY;
            printf("Selected role: SPY\n");
            break;
        case BTN_TEAM_RED:
            context->player_team = TEAM_RED;
            printf("Selected team: RED\n");
            break;
        case BTN_TEAM_BLUE:
            context->player_team = TEAM_BLUE;
            printf("Selected team: BLUE\n");
            break;
        case BTN_QUIT:
            return BTN_RET_QUIT;
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
    (void)context;
    if (lobby_buttons_created) return 0;
    /* créer les boutons de rôle/équipe (text_button_create existe déjà dans ton projet) */
    SDL_Color white = {255,255,255,255};
    /* positions relatives — ajuste selon ton layout */
    int cx = WIN_WIDTH/2;
    text_button_create(context->renderer, BTN_AGENT, cx - 300, 400, 80, "Agent", FONT_LARABIE, white, lobby_button_click);
    text_button_create(context->renderer, BTN_SPY,   cx - 120, 400, 80, "Espion", FONT_LARABIE, white, lobby_button_click);
    text_button_create(context->renderer, BTN_TEAM_RED,  cx + 60, 400, 80, "Equipe Rouge", FONT_LARABIE, white, lobby_button_click);
    text_button_create(context->renderer, BTN_TEAM_BLUE, cx + 260, 400, 80, "Equipe Bleue", FONT_LARABIE, white, lobby_button_click);
    lobby_buttons_created = 1;
    /* Pas de ressources supplémentaires à charger pour l'instant.
       Si besoin : charger textures, sons, etc. ici. */
    return 0;
}

void lobby_display(SDL_Context* context) {
    if (!context) return;

    /* Si on n'est pas dans un lobby, rien à afficher ici */
    if (context->lobby_id == -1) return;

    /* Masquer / afficher les boutons pertinents */
    hide_button(BTN_CREATE);
    hide_button(BTN_JOIN);
    show_button(BTN_QUIT);
    /* afficher les boutons de rôle/équipe */
    show_button(BTN_AGENT);
    show_button(BTN_SPY);
    show_button(BTN_TEAM_RED);
    show_button(BTN_TEAM_BLUE);

    /* Afficher le code et l'id du lobby */
    char buf[128];
    format_to(buf, sizeof(buf), "Lobby %d  •  Code : %s",
              context->lobby_id,
              context->lobby_code ? context->lobby_code : "----");

    /* Affichage centré horizontalement, position verticale relative */
    int rel_x = 0; /* 0 = centré (convention utilisée dans text_display) */
    int rel_y = -200;
    text_display(context->renderer, buf, FONT_LARABIE, 36, COL_WHITE, rel_x, rel_y, 0, 255);

    /* Si tu veux, afficher la liste des joueurs ici (nécessite que le client reçoive la liste du serveur). */
}

void lobby_handle_event(SDL_Context* context, SDL_Event* e) {
    if (!context || !e) return;

    /* Laisser le système de boutons traiter l'événement */
    ButtonReturn ret = buttons_handle_event(context, e);
    if (ret == BTN_RET_QUIT) {
        /* Quitter le lobby localement : repasser en menu et nettoyer le code */
        context->lobby_id = -1;
        if (context->lobby_code) {
            free(context->lobby_code);
            context->lobby_code = NULL;
        }
        context->game_state = GAME_STATE_MENU;
        /* Masquer les boutons de rôle */
        hide_button(BTN_AGENT);
        hide_button(BTN_SPY);
        hide_button(BTN_TEAM_RED);
        hide_button(BTN_TEAM_BLUE);

        /* Optionnel : prévenir le serveur qu'on quitte le lobby (ajouter message si le protocole gère cela) */
    }
}

void lobby_free(){
    /* Rien à libérer pour l'instant; si on charge des ressources locales, les détruire ici. */
}