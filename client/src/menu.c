#include "../lib/all.h"

SDL_Texture* menu_logo;
SDL_Texture* quagmire;
Button* btn_create;
Button* btn_join;
Button* btn_quit;
Button* btn_social;
Input* name_input = NULL;
Input* code_input = NULL;
char* name = NULL; // Le nom du joueur, à envoyer au serveur lors de la création ou du join d'un lobby. Peut être NULL ou chaîne vide si non défini.
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
        write_property("PLAYER_NAME", text);
    } else {
        name = NULL;
        write_property("PLAYER_NAME", "");
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

ButtonReturn menu_handle_event(SDL_Context* context, SDL_Event* e) {
    if (name_input) input_handle_event(context, name_input, e);
    if (code_input && joining) input_handle_event(context, code_input, e);

    ButtonReturn ret = BTN_RET_NONE;
    ButtonReturn r;
    if (btn_create) { r = button_handle_event(context, btn_create, e); if (r != BTN_RET_NONE) ret = r; }
    if (btn_join && !joining) { r = button_handle_event(context, btn_join, e); if (r != BTN_RET_NONE) ret = r; }
    if (btn_quit)   { r = button_handle_event(context, btn_quit, e);   if (r != BTN_RET_NONE) ret = r; }
    return ret;
}

ButtonReturn menu_button_click(SDL_Context* context, Button* button) {
    if (button == btn_join) {
        joining = 1;
    } else if (button == btn_create) {
        char trame[20];
        format_to(trame, sizeof(trame), "%d %s", MSG_CREATELOBBY, name ? name : "NONE");
        send_tcp(context->sock, trame);
    } else if (button == btn_quit) {
        return BTN_RET_QUIT;
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
        btn_create = button_create(context->renderer, 0, cfg_btn_create);
        free(cfg_btn_create);
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
        btn_join = button_create(context->renderer, 0, cfg_btn_join);
        free(cfg_btn_join);
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
        btn_quit = button_create(context->renderer, 0, cfg_btn_quit);
        free(cfg_btn_quit);
    }

    ButtonConfig* cfg_btn_social = button_config_init();
    if (cfg_btn_social) {
        cfg_btn_social->x         = 0;
        cfg_btn_social->y         = -200;
        cfg_btn_social->h         = 100;
        cfg_btn_social->font_path = FONT_LARABIE;
        cfg_btn_social->color     = COL_WHITE;
        cfg_btn_social->text      = "Social";
        cfg_btn_social->callback  = menu_button_click;
        btn_social = button_create(context->renderer, 0, cfg_btn_social);
        free(cfg_btn_social);
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
        free(cfg_in_name);
    }

    if (name_input) {
        const char* loaded_name = input_get_text(name_input);
        if (loaded_name && loaded_name[0] != '\0') {
            input_submit(context, name_input);
        }
    }

    /* Si aucun nom n'a été chargé depuis la sauvegarde, en choisir un
       aléatoirement dans assets/misc/usernames.txt et l'appliquer. */
    if (!name || name[0] == '\0') {
        char saved[INPUT_DEFAULT_MAX + 1] = {0};
        int has_saved = (read_property(saved, "PLAYER_NAME") == EXIT_SUCCESS && saved[0] != '\0');
        if (!has_saved) {
            FILE* uf = fopen("assets/misc/usernames.txt", "r");
            if (uf) {
                /* Compter les lignes */
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
                            /* Retirer le '\n' final */
                            size_t len = strlen(line_buf);
                            if (len > 0 && line_buf[len - 1] == '\n') line_buf[len - 1] = '\0';
                            /* Appliquer le nom via name_on_submit */
                            name_on_submit(context, line_buf);
                            /* Mettre à jour l'input pour que le label "Pseudo : ..."
                               s'affiche immédiatement au premier lancement. */
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
        cfg_in_code->submitted_label = "Rejoindre : ";
        cfg_in_code->maxlen = 5;
        cfg_in_code->centered = 1;
        cfg_in_code->on_submit = code_on_submit;
        cfg_in_code->allowed_pattern = "^[0-9]$";
        cfg_in_code->submit_pattern = "^[0-9]{5}$";
        cfg_in_code->bg_path = "assets/img/inputs/empty.png";
        cfg_in_code->bg_padding = 24;
        code_input = input_create(context->renderer, INPUT_ID_JOIN_CODE, cfg_in_code);
        free(cfg_in_code);
    }

    return loading_fails;
}

void menu_display(SDL_Context * context) {

    if (context->lobby->id != -1) {
        context->app_state = APP_STATE_LOBBY;
        joining = 0;
    }


    if (!audio_is_playing(MUSIC_MENU_LOBBY)) {
        audio_play(MUSIC_MENU_LOBBY, -1);
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

    button_render(context->renderer, btn_create);
    button_render(context->renderer, btn_quit);

    if (joining) input_render(context->renderer, code_input);
    else button_render(context->renderer, btn_join);

}

int menu_free() {
    if (menu_logo) { free_image(menu_logo); menu_logo = NULL; }
    if (quagmire) { free_image(quagmire); quagmire = NULL; }

    if (btn_create) { button_destroy(btn_create); btn_create = NULL; }
    if (btn_join) { button_destroy(btn_join); btn_join = NULL; }
    if (btn_quit) { button_destroy(btn_quit); btn_quit = NULL; }
    if (btn_social) { button_destroy(btn_social); btn_social = NULL; }

    if (name_input) { input_destroy(name_input); name_input = NULL; }
    if (code_input) { input_destroy(code_input); code_input = NULL; }

    if (name) { free(name); name = NULL; }

    joining = 0;

    return EXIT_SUCCESS;
}