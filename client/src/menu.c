#include "../lib/all.h"

/* Ressources UI du module menu */
static SDL_Texture* menu_logo = NULL;
static Button* btn_create = NULL;
static Button* btn_join = NULL;
static Button* btn_quit = NULL;
static Button* btn_social = NULL;
static Button* btn_tuto = NULL;
static Input* name_input = NULL;
static Input* code_input = NULL;
static int joining = 0;

static const char* NAME_PLACEHOLDERS[] = {"Peter", "Quagmire", "Tom", "Faz Faf"};
static const char* CODE_PLACEHOLDERS[] = {"CODE : #####"};

static void name_on_submit(AppContext* context, const char* text) {
	if (!context) return;

    printf("Name input submitted: %s\n", text ? text : "");
    if (context->player_name) {
        free(context->player_name);
        context->player_name = NULL;
    }
    if (text) {
        context->player_name = strdup(text);
        if (!context->player_name) {
            context->player_name = malloc(1);
            if (context->player_name) context->player_name[0] = '\0';
        }
        write_property("PLAYER_NAME", text);
    } else {
        context->player_name = NULL;
        write_property("PLAYER_NAME", "");
    }
}

static void code_on_submit(AppContext* context, const char* text) {
	if (!context) return;

    printf("Code input submitted: %s\n", text ? text : "");
    if (text) {
        char trame[20];
        format_to(trame, sizeof(trame), "%d %s %s", MSG_JOINLOBBY, text, context->player_name ? context->player_name : "NONE");
        send_tcp(context->sock, trame);
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;
    }
}

static ButtonReturn menu_button_click(AppContext* context, Button* button) {
	if (!context || !button) return BTN_NONE;

    if (button == btn_join) {
        joining = 1;
    } else if (button == btn_tuto) {
        tuto_open();
    } else if (button == btn_create) {
        char trame[20];
        format_to(trame, sizeof(trame), "%d %s", MSG_CREATELOBBY, context->player_name ? context->player_name : "NONE");
        send_tcp(context->sock, trame);
        context->player_role = ROLE_NONE;
        context->player_team = TEAM_NONE;
    } else if (button == btn_quit) {
        return BTN_MENU_QUIT;
    }
    return BTN_NONE;
}

static Button* menu_create_button(AppContext* context, int id, int x, int y, int h, const char* text) {
    if (!context) return NULL;

    ButtonConfig* cfg = button_config_init();
    if (!cfg) return NULL;

    cfg->x = x;
    cfg->y = y;
    cfg->h = h;
    cfg->font_path = FONT_LARABIE;
    cfg->color = COL_WHITE;
    cfg->text = text;
    cfg->callback = menu_button_click;

    Button* created = button_create(context->renderer, id, cfg);
    free(cfg);
    return created;
}

static int menu_init_buttons(AppContext* context) {
    int loading_fails = 0;

    btn_create = menu_create_button(context, BTN_CREATE_LOBBY, -300, -180, 100, "Créer");
    if (!btn_create) loading_fails++;

    btn_join = menu_create_button(context, BTN_JOIN_LOBBY, 300, -180, 100, "Rejoindre");
    if (!btn_join) loading_fails++;

    btn_quit = menu_create_button(context, BTN_MENU_QUIT, 0, -400, 100, "Quitter");
    if (!btn_quit) loading_fails++;

    btn_social = menu_create_button(context, BTN_MENU_SOCIAL, 775, -350, 55, "Social");
    if (!btn_social) loading_fails++;

    btn_tuto = menu_create_button(context, BTN_MENU_TUTO, 775, -400, 55, "Tutoriel");
    if (!btn_tuto) loading_fails++;

    return loading_fails;
}

static int menu_init_name_input(AppContext* context) {
    if (!context) return EXIT_FAILURE;

    InputConfig* cfg_in_name = input_config_init();
    if (!cfg_in_name) return EXIT_FAILURE;

    cfg_in_name->x = 775;
    cfg_in_name->y = 450;
    cfg_in_name->w = 250;
    cfg_in_name->h = 60;
    cfg_in_name->font_path = FONT_LARABIE;
    cfg_in_name->font_size = 28;
    cfg_in_name->placeholders = NAME_PLACEHOLDERS;
    cfg_in_name->placeholder_count = 4;
    cfg_in_name->submitted_label = "Pseudo : ";
    cfg_in_name->maxlen = 16;
    cfg_in_name->save_player_data = 1;
    cfg_in_name->on_submit = name_on_submit;
    cfg_in_name->allowed_pattern = "^[a-zA-Z0-9_zéèêëàâäåæçîïìùûüÿœ]*$";
    cfg_in_name->submit_pattern = "^[a-zA-Z0-9_zéèêëàâäåæçîïìùûüÿœ]{3,16}$";
    cfg_in_name->bg_path = "assets/img/inputs/empty.png";
    cfg_in_name->bg_padding = 24;

    name_input = input_create(context->renderer, INPUT_NAME, cfg_in_name);
    free(cfg_in_name);

    return name_input ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int menu_init_code_input(AppContext* context) {
    if (!context) return EXIT_FAILURE;

    InputConfig* cfg_in_code = input_config_init();
    if (!cfg_in_code) return EXIT_FAILURE;

    cfg_in_code->x = 300;
    cfg_in_code->y = -180;
    cfg_in_code->w = 385;
    cfg_in_code->h = 100;
    cfg_in_code->font_path = FONT_LARABIE;
    cfg_in_code->font_size = 28;
    cfg_in_code->placeholders = CODE_PLACEHOLDERS;
    cfg_in_code->placeholder_count = 1;
    cfg_in_code->submitted_label = "Rejoindre : ";
    cfg_in_code->maxlen = 5;
    cfg_in_code->centered = 1;
    cfg_in_code->on_submit = code_on_submit;
    cfg_in_code->allowed_pattern = "^[0-9]$";
    cfg_in_code->submit_pattern = "^[0-9]{5}$";
    cfg_in_code->submit_sound = "assets/audio/sfx/input/submit.ogg";
    cfg_in_code->bg_path = "assets/img/inputs/empty.png";
    cfg_in_code->bg_padding = 24;

    code_input = input_create(context->renderer, INPUT_JOIN_CODE, cfg_in_code);
    free(cfg_in_code);

    return code_input ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void menu_apply_random_player_name_if_needed(AppContext* context) {
    if (!context) return;
    if (context->player_name && context->player_name[0] != '\0') return;

    char saved[INPUT_DEFAULT_MAX + 1] = {0};
    int has_saved = (read_property(saved, "PLAYER_NAME") == EXIT_SUCCESS && saved[0] != '\0');
    if (has_saved) return;

    FILE* uf = fopen("assets/misc/usernames.txt", "r");
    if (!uf) return;

    int line_count = 0;
    char line_buf[128];
    while (fgets(line_buf, sizeof(line_buf), uf)) line_count++;

    if (line_count > 0) {
        srand((unsigned int)SDL_GetTicks());
        int chosen = rand() % line_count;

        rewind(uf);
        int cur = 0;
        while (fgets(line_buf, sizeof(line_buf), uf)) {
            if (cur == chosen) {
                size_t len = strlen(line_buf);
                if (len > 0 && line_buf[len - 1] == '\n') line_buf[len - 1] = '\0';

                name_on_submit(context, line_buf);
                if (name_input) {
                    input_set_text(name_input, line_buf);
                    input_submit(context, name_input);
                }
                break;
            }
            cur++;
        }
    }

    fclose(uf);
}

ButtonReturn menu_handle_event(AppContext* context, SDL_Event* e) {
    if (!context || !e) return BTN_NONE;

    if (tuto_is_active()) {
        tuto_handle_event(context, e);
        return BTN_NONE;
    }

    if (name_input) input_handle_event(context, name_input, e);
    if (code_input && joining) input_handle_event(context, code_input, e);

    ButtonReturn ret = BTN_NONE;
    ButtonReturn r = BTN_NONE;

    if (btn_create) {
        r = button_handle_event(context, btn_create, e);
        if (r != BTN_NONE) ret = r;
    }
    if (btn_join && !joining) {
        r = button_handle_event(context, btn_join, e);
        if (r != BTN_NONE) ret = r;
    }
    if (btn_quit) {
        r = button_handle_event(context, btn_quit, e);
        if (r != BTN_NONE) ret = r;
    }
    if (btn_tuto) {
        r = button_handle_event(context, btn_tuto, e);
        if (r != BTN_NONE) ret = r;
    }

    return ret;
}

int menu_init(AppContext* context) {
    if (!context) return EXIT_FAILURE;

    int loading_fails = 0;

    menu_logo = load_image(context->renderer, "assets/img/others/logo_titre.png");
    if (!menu_logo) {
        printf("Failed to load menu logo image\n");
        loading_fails++;
    }

    loading_fails += menu_init_buttons(context);

    if (menu_init_name_input(context) != EXIT_SUCCESS) {
        loading_fails++;
    }

    if (name_input) {
        const char* loaded_name = input_get_text(name_input);
        if (loaded_name && loaded_name[0] != '\0') {
            input_submit(context, name_input);
        }
    }

    menu_apply_random_player_name_if_needed(context);

    if (name_input) {
        printf("Setting submit sound for name input\n");
        edit_in_cfg(INPUT_NAME, IN_CFG_SUBMIT_SOUND, (intptr_t)"assets/audio/sfx/input/submit.ogg");
    }

    if (menu_init_code_input(context) != EXIT_SUCCESS) {
        loading_fails++;
    }

    if (tuto_init(context) != EXIT_SUCCESS) {
        printf("Failed to initialize tutorial\n");
        loading_fails++;
    }

    return loading_fails;
}

void menu_display(AppContext* context) {
    if (!context || !context->lobby) return;

    if (context->lobby->id != -1) {
        context->app_state = APP_STATE_LOBBY;
        joining = 0;
    }

    if (!audio_is_playing(MUSIC_MENU_LOBBY)) {
        audio_play_with_fade(MUSIC_MENU_LOBBY, -1, 1500, AUDIO_FADE_IN_BY_VOLUME, NULL);
    }

    if (menu_logo) {
        display_image(context->renderer, menu_logo, 0, 200, 1.00, 0, SDL_FLIP_NONE, 1, 255);
    }

    if (name_input) input_render(context->renderer, name_input);

    if (btn_create) button_render(context->renderer, btn_create);
    if (btn_quit) button_render(context->renderer, btn_quit);
    if (btn_tuto) button_render(context->renderer, btn_tuto);

    if (joining) {
        if (code_input) input_render(context->renderer, code_input);
    } else if (btn_join) {
        button_render(context->renderer, btn_join);
    }

    if (tuto_is_active()) {
        tuto_display(context);
    }
}

int menu_free() {
    if (menu_logo) {
        free_image(menu_logo);
        menu_logo = NULL;
    }

    if (btn_create) {
        button_destroy(btn_create);
        btn_create = NULL;
    }
    if (btn_join) {
        button_destroy(btn_join);
        btn_join = NULL;
    }
    if (btn_quit) {
        button_destroy(btn_quit);
        btn_quit = NULL;
    }
    if (btn_social) {
        button_destroy(btn_social);
        btn_social = NULL;
    }
    if (btn_tuto) {
        button_destroy(btn_tuto);
        btn_tuto = NULL;
    }

    if (name_input) {
        input_destroy(name_input);
        name_input = NULL;
    }
    if (code_input) {
        input_destroy(code_input);
        code_input = NULL;
    }

    tuto_free();

    joining = 0;

    return EXIT_SUCCESS;
}