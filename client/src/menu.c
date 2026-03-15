#include "../lib/all.h"

SDL_Texture* menu_logo;
SDL_Texture* quagmire;
Button* btn_create;
Button* btn_join;
Button* btn_quit;
Input* name_input = NULL;
Input* code_input = NULL;
char* name = NULL;
int joining = 0;

static void name_on_submit(SDL_Context* context, const char* text) {
    printf("Name input submitted: %s\n", text ? text : "");
    if (name) {
        free(name);
        name = NULL;
    }
    if (text) {
        name = strdup(text);
        if (!name) {
            name = malloc(1);
            if (name) name[0] = '\0';
        }
    } else {
        name = NULL;
    }
}

static void code_on_submit(SDL_Context* context, const char* text) {
    printf("Code input submitted: %s\n", text ? text : "");
    if (text) {
        char trame[20];
        format_to(trame, sizeof(trame), "%d %s %s", MSG_JOINLOBBY, text, name ? name : "NONE");
        send_tcp(context->sock, trame);
    }
}

void menu_handle_event(SDL_Context* context, SDL_Event* e) {
    if (name_input) input_handle_event(context, name_input, e);
    if (code_input) input_handle_event(context, code_input, e);
}

ButtonReturn menu_button_click(SDL_Context* context, ButtonId button_id) {
    printf("Button clicked: %d\n", button_id);
    switch (button_id) {
        case BTN_JOIN: joining = 1; break;
        case BTN_CREATE:
            char trame[20];
            format_to(trame, sizeof(trame), "%d %s", MSG_CREATELOBBY, name ? name : "NONE");
            send_tcp(context->sock, trame);
            break;
        case BTN_QUIT: return BTN_RET_QUIT; break;
        default: printf("Unknown button clicked\n"); break;
    }
    return BTN_RET_NONE;
}

int menu_init(SDL_Context * context) {
    int loading_fails = 0;

    // Chargement image
    menu_logo = load_image(context->renderer, "assets/img/others/logo_titre.png");
    if (!menu_logo) {
        printf("Failed to load menu logo image\n");
        loading_fails++;
    }
    quagmire = load_image(context->renderer, "assets/img/others/quagmire.png");
    if (!quagmire) {
        printf("Failed to load quagmire image\n");
        loading_fails++;
    }

    // Chargement boutons
    ButtonConfig* cfg_btn_create = button_config_init();
    if (cfg_btn_create) {
        cfg_btn_create->x         = -300;
        cfg_btn_create->y         = -180;
        cfg_btn_create->h         = 100;
        cfg_btn_create->font_path = FONT_LARABIE;
        cfg_btn_create->color     = COL_WHITE;
        cfg_btn_create->text      = "Créer";
        cfg_btn_create->callback  = menu_button_click;
        btn_create = button_create(context->renderer, BTN_CREATE, cfg_btn_create);
    }

    ButtonConfig* cfg_btn_join = button_config_init();
    if (cfg_btn_join) {
        cfg_btn_join->x         = 300;
        cfg_btn_join->y         = -180;
        cfg_btn_join->h         = 100;
        cfg_btn_join->font_path = FONT_LARABIE;
        cfg_btn_join->color     = COL_WHITE;
        cfg_btn_join->text      = "Rejoindre";
        cfg_btn_join->callback  = menu_button_click;
        btn_join = button_create(context->renderer, BTN_JOIN, cfg_btn_join);
    }

    ButtonConfig* cfg_btn_quit = button_config_init();
    if (cfg_btn_quit) {
        cfg_btn_quit->x         = 0;
        cfg_btn_quit->y         = -400;
        cfg_btn_quit->h         = 100;
        cfg_btn_quit->font_path = FONT_LARABIE;
        cfg_btn_quit->color     = COL_WHITE;
        cfg_btn_quit->text      = "Quitter";
        cfg_btn_quit->callback  = menu_button_click;
        btn_quit = button_create(context->renderer, BTN_QUIT, cfg_btn_quit);
    }

    // Chargement input
    static const char* name_placeholders[] = {"Peter", "Quagmire", "Tom", "Faz Faf"};

    InputConfig* cfg_in_name = input_config_init();
    if (cfg_in_name) {
        cfg_in_name->x = 775;
        cfg_in_name->y = 450;
        cfg_in_name->w = 250;
        cfg_in_name->h = 60;
        cfg_in_name->font_path = FONT_LARABIE;
        cfg_in_name->font_size = 28;
        cfg_in_name->placeholders = name_placeholders;
        cfg_in_name->placeholder_count = 4;
        cfg_in_name->submitted_label = "Pseudo : ";
        cfg_in_name->maxlen = 16;
        cfg_in_name->save_player_data = 1;
        cfg_in_name->on_submit = name_on_submit;
        cfg_in_name->bg_path = "assets/img/inputs/empty.png";
        cfg_in_name->bg_padding = 24;
        name_input = input_create(context->renderer, INPUT_ID_NAME, cfg_in_name);
    }

    if (name_input) {
        const char* loaded_name = input_get_text(name_input);
        if (loaded_name && loaded_name[0] != '\0') {
            input_submit(context, name_input);
        }
    }

    static const char* code_placeholders[] = {"CODE"};
    InputConfig* cfg_in_code = input_config_init();
    if (cfg_in_code) {
        cfg_in_code->x = 300;
        cfg_in_code->y = -180;
        cfg_in_code->w = 385;
        cfg_in_code->h = 100;
        cfg_in_code->font_path = FONT_LARABIE;
        cfg_in_code->font_size = 28;
        cfg_in_code->placeholders = code_placeholders;
        cfg_in_code->placeholder_count = 1;
        cfg_in_code->submitted_label = "";
        cfg_in_code->maxlen = 16;
        cfg_in_code->centered = 1;
        cfg_in_code->on_submit = code_on_submit;
        cfg_in_code->allowed_pattern = "^[0-9]$";
        cfg_in_code->bg_path = "assets/img/inputs/empty.png";
        cfg_in_code->bg_padding = 24;
        code_input = input_create(context->renderer, INPUT_ID_JOIN_CODE, cfg_in_code);
    }

    return loading_fails;
}

void menu_display(SDL_Context * context) {

    if (context->lobby_id == -1) {
        context->game_state = GAME_STATE_MENU;
            edit_btn_cfg(BTN_CREATE, BTN_CFG_HIDDEN, 0);
            edit_btn_cfg(BTN_JOIN, BTN_CFG_HIDDEN, 0);
            edit_btn_cfg(BTN_QUIT, BTN_CFG_HIDDEN, 0);
            edit_btn_cfg(BTN_RED_AGENT, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_RED_SPY, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_BLUE_AGENT, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_BLUE_SPY, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_RETURN, BTN_CFG_HIDDEN, 1);
    } else {
        context->game_state = GAME_STATE_LOBBY;
            edit_btn_cfg(BTN_CREATE, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_JOIN, BTN_CFG_HIDDEN, 1);
            edit_btn_cfg(BTN_QUIT, BTN_CFG_HIDDEN, 1);
    }

    if (!audio_is_playing(MUSIC_MENU)) {
        audio_play(MUSIC_MENU, -1);
    }
   
    if (context->game_state == GAME_STATE_MENU) {
        if (joining) {
            edit_btn_cfg(BTN_CREATE, BTN_CFG_HIDDEN, 0);
            edit_btn_cfg(BTN_JOIN, BTN_CFG_HIDDEN, 1);
            if (code_input) input_render(context->renderer, code_input);
        } else {
            edit_btn_cfg(BTN_CREATE, BTN_CFG_HIDDEN, 0);
            edit_btn_cfg(BTN_JOIN, BTN_CFG_HIDDEN, 0);
        }
    }

    // Afficher le logo à sa taille d'origine
    if (menu_logo) {
        display_image(context->renderer, menu_logo, 0, 200, 1.00, 0, SDL_FLIP_NONE, 1, 255);
    }
    
    // Afficher quagmire à sa taille d'origine
    if (quagmire) {
        display_image(context->renderer, quagmire, 850, -350, 1.0, 0, SDL_FLIP_NONE, 1, 255);
    }

    /* input is drawn by menu */
    if (name_input) input_render(context->renderer, name_input);

    /* submitted text is drawn by input_render */
}

int menu_free() {
    if (menu_logo) free_image(menu_logo);
    if (quagmire) free_image(quagmire);
    btn_create = NULL;
    btn_join = NULL;
    btn_quit = NULL;
    if (name_input) input_destroy(name_input);
    if (code_input) input_destroy(code_input);
    name_input = NULL;
    code_input = NULL;

    return EXIT_SUCCESS;
}