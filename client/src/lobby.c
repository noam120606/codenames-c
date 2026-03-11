#include "../lib/all.h"

static SDL_Texture* lobby_btn_red = NULL;
static SDL_Texture* lobby_btn_blue = NULL;

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
            printf("Selected team: BLUE AGENT\n");
            break;
        case BTN_BLUE_SPY:
            context->player_role = ROLE_SPY;
            context->player_team = TEAM_BLUE;
            printf("Selected team: BLUE SPY\n");
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

    /* créer les boutons de rôle/équipe (text_button_create existe déjà dans ton projet)
       Positionner centrés et espacés verticalement pour être visibles sur la plupart des écrans */
    int cx = WIN_WIDTH / 2;
    int left_x = cx - 360;   /* colonne gauche (rouge) */
    int right_x = cx + 200;  /* colonne droite (bleu) */
    int top_y = WIN_HEIGHT / 2 - 120;    /* agent (haut) */
    int bottom_y = WIN_HEIGHT / 2 + 40;  /* espion (bas) */
    int height = 64;

    /* gauche = équipe rouge : Agent (haut) / Espion (bas) */
    text_button_create(context->renderer, BTN_RED_AGENT,  left_x,  top_y, height, "Agent rouge",  FONT_LARABIE, COL_WHITE, lobby_button_click);
    text_button_create(context->renderer, BTN_RED_SPY, left_x, bottom_y, height, "Espion rouge", FONT_LARABIE, COL_WHITE, lobby_button_click);

    /* droite = équipe bleue : Agent (haut) / Espion (bas) */
    text_button_create(context->renderer, BTN_BLUE_AGENT, right_x, top_y, height, "Agent bleu", FONT_LARABIE, COL_WHITE, lobby_button_click);
    text_button_create(context->renderer, BTN_BLUE_SPY, right_x, bottom_y, height, "Espion bleu", FONT_LARABIE, COL_WHITE, lobby_button_click);

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
    show_button(BTN_RED_AGENT);
    show_button(BTN_RED_SPY);
    show_button(BTN_BLUE_AGENT);
    show_button(BTN_BLUE_SPY);

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
        hide_button(BTN_RED_AGENT);
        hide_button(BTN_RED_SPY);
        hide_button(BTN_BLUE_AGENT);
        hide_button(BTN_BLUE_SPY);

        /* Optionnel : prévenir le serveur qu'on quitte le lobby (ajouter message si le protocole gère cela) */
    }
}

int lobby_free(){
    /* détruire textures d'assets chargées par lobby */

    if (lobby_btn_red) {
        SDL_DestroyTexture(lobby_btn_red);
        lobby_btn_red = NULL;
    }
    if (lobby_btn_blue) {
        SDL_DestroyTexture(lobby_btn_blue);
        lobby_btn_blue = NULL;
    }
    buttons_free();
    return EXIT_SUCCESS;
}