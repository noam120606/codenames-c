#include "../lib/all.h"

SDL_Texture* bandeau;

// Crossfaders pour le volume
static Crossfader* cf_music = NULL;
static Crossfader* cf_sfx = NULL;

// Texte des règles dans le bandeau (chargé depuis rules.txt)
static const char* rules_title = "RÈGLES DU JEU";
static char** rules_lines = NULL;
static int rules_line_count = 0;

// Textes statiques et dynamiques pour le nouveau système optimisé
static Text* txt_music_label = NULL;
static Text* txt_sfx_label = NULL;
static Text* txt_rules_title = NULL;
static Text** txt_rules_lines = NULL;
static Text* txt_fps = NULL;
static Text* txt_ping = NULL;
static Text* txt_server_local = NULL;
static Text* txt_server_distant = NULL;
static Text* txt_bar_1 = NULL;
static Text* txt_bar_2 = NULL;
static Text* txt_version = NULL;

// Animation state
typedef enum {
    INFOS_HIDDEN,
    INFOS_SHOWING,
    INFOS_VISIBLE,
    INFOS_HIDING
} InfosState;

static InfosState infos_state = INFOS_HIDDEN;
static Uint32 animation_start_time = 0;
static const Uint32 ANIMATION_DURATION = 200; // Durée de l'animation en ms

// Position courante du bandeau (mise à jour chaque frame)
static int current_display_x = -1300;

/* Callback appelé quand le crossfader musique change */
static void on_music_volume_change(AppContext* ctx, int new_value) {
    if (ctx) ctx->music_volume = new_value;
    audio_set_type_volume(AUDIO_SOUND_KIND_MUSIC, new_value);
}

/* Callback appelé quand le crossfader effets sonores change */
static void on_sfx_volume_change(AppContext* ctx, int new_value) {
    if (ctx) ctx->sound_effects_volume = new_value;
    audio_set_type_volume(AUDIO_SOUND_KIND_SFX, new_value);
}

/* Charge les règles depuis le fichier rules.txt */
static void load_rules() {
    FILE* f = fopen("assets/misc/rules.txt", "r");
    if (!f) {
        printf("Failed to open rules.txt\n");
        return;
    }

    char line_buf[256];
    while (fgets(line_buf, sizeof(line_buf), f)) {
        // Retirer le '\n' final
        size_t len = strlen(line_buf);
        if (len > 0 && line_buf[len - 1] == '\n') line_buf[len - 1] = '\0';

        // Allouer et copier la ligne
        char* line_copy = strdup(line_buf);
        if (!line_copy) continue;

        // Agrandir le tableau
        char** tmp = realloc(rules_lines, sizeof(char*) * (rules_line_count + 1));
        if (!tmp) {
            free(line_copy);
            continue;
        }
        rules_lines = tmp;
        rules_lines[rules_line_count] = line_copy;
        rules_line_count++;
    }

    fclose(f);
}

/* Libère la mémoire des règles */
static void free_rules() {
    if (rules_lines) {
        for (int i = 0; i < rules_line_count; i++) {
            free(rules_lines[i]);
        }
        free(rules_lines);
        rules_lines = NULL;
    }
    rules_line_count = 0;
}

int init_infos(AppContext* context) {
    int loading_fails = 0;

    /* Charger les règles depuis le fichier */
    load_rules();

    bandeau = load_image(context->renderer, "assets/img/others/bandeau_infos.png");
    if (!bandeau) {
        printf("Failed to load bandeau image\n");
        loading_fails++;
    }

    /* Initialiser le sous-système crossfader */
    crossfaders_init();

    /* Crossfader musique */
    CrossfaderConfig* cfg_music = crossfader_config_init();
    if (cfg_music) {
        cfg_music->x = 0;  /* repositionné à chaque frame */
        cfg_music->y = 320;
        cfg_music->w = 200;
        cfg_music->h = 16;
        cfg_music->min = 0;
        cfg_music->max = MIX_MAX_VOLUME;
        cfg_music->value = context->music_volume;
        cfg_music->save_player_data = 1;
        cfg_music->color_0_pct = (SDL_Color){90, 90, 200, 220};
        cfg_music->color_100_pct = (SDL_Color){200, 110, 90, 220};
        cfg_music->knob_color = (SDL_Color){0, 200, 220, 255};
        cfg_music->hidden = 1;
        cfg_music->on_change = on_music_volume_change;
        cf_music = crossfader_create(context->renderer, CROSSFADER_ID_MUSIC_VOLUME, cfg_music);
        free(cfg_music);
    }

    if (!cf_music) {
        printf("Failed to create music volume crossfader\n");
        loading_fails++;
    } else if (context) {
        context->music_volume = cf_music->cfg->value;
    }

    /* Crossfader effets sonores */
    CrossfaderConfig* cfg_sfx = crossfader_config_init();
    if (cfg_sfx) {
        cfg_sfx->x = 0;  /* repositionné à chaque frame */
        cfg_sfx->y = 250;
        cfg_sfx->w = 200;
        cfg_sfx->h = 16;
        cfg_sfx->min = 0;
        cfg_sfx->max = MIX_MAX_VOLUME;
        cfg_sfx->value = context->sound_effects_volume;
        cfg_sfx->save_player_data = 1;
        cfg_sfx->color_0_pct = (SDL_Color){90, 90, 200, 220};
        cfg_sfx->color_100_pct = (SDL_Color){200, 110, 90, 220};
        cfg_sfx->knob_color = (SDL_Color){0, 200, 220, 255};
        cfg_sfx->hidden = 1;
        cfg_sfx->on_change = on_sfx_volume_change;
        cf_sfx = crossfader_create(context->renderer, CROSSFADER_ID_SFX_VOLUME, cfg_sfx);
        free(cfg_sfx);
    }

    if (!cf_sfx) {
        printf("Failed to create sfx volume crossfader\n");
        loading_fails++;
    } else if (context) {
        context->sound_effects_volume = cf_sfx->cfg->value;
    }

    if (context) {
        audio_set_type_volume(AUDIO_SOUND_KIND_MUSIC, context->music_volume);
        audio_set_type_volume(AUDIO_SOUND_KIND_SFX, context->sound_effects_volume);
    }

    

    /* Initialisation des textes optimisés */
    // Textes statiques (labels)
    txt_music_label = init_text(context, "Musique", 
        create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 345, 0, 255));
    txt_sfx_label = init_text(context, "Effets Sonores", 
        create_text_config(FONT_LARABIE, 18, COL_WHITE, 0, 275, 0, 255));
    
    // Titre et lignes des règles
    txt_rules_title = init_text(context, rules_title, 
        create_text_config(FONT_LARABIE, 20, COL_WHITE, 0, 180, 0, 255));
    
    if (rules_line_count > 0) {
        txt_rules_lines = malloc(sizeof(Text*) * rules_line_count);
        for (int i = 0; i < rules_line_count; i++) {
            txt_rules_lines[i] = init_text(context, rules_lines[i], 
                create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 180 - 35 - (i * 22), 0, 200));
        }
    }
    
    // Textes dynamiques (FPS, ping, etc.)
    txt_fps = init_text(context, "FPS : 0.00", 
        create_text_config(FONT_LARABIE, 22, COL_WHITE, 0, 450, 0, 255));
    txt_ping = init_text(context, "Ping : --", 
        create_text_config(FONT_LARABIE, 22, COL_WHITE, 0, 410, 0, 255));
    
    // Textes serveur
    txt_server_local = init_text(context, "(serveur local)", 
        create_text_config(FONT_LARABIE, 14, COL_GREEN, 0, 385, 0, 220));
    txt_server_distant = init_text(context, "(serveur distant)", 
        create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, 385, 0, 220));
    
    // Barres décoratives
    txt_bar_1 = init_text(context, "______________", 
        create_text_config(FONT_LARABIE, 22, COL_WHITE, 0, 370, 0, 255));
    txt_bar_2 = init_text(context, "______________", 
        create_text_config(FONT_LARABIE, 22, COL_WHITE, 0, 230, 0, 255));

    /* Texte de version */
    if (context && context->version) {
        txt_version = init_text(context, context->version,
            create_text_config(FONT_LARABIE, 14, COL_WHITE, 0, -500, 0, 255));
    }

    return loading_fails;
}

// Démarre l'animation d'apparition
void infos_display_show_animation(AppContext* context) {
    if (infos_state != INFOS_VISIBLE && infos_state != INFOS_SHOWING) {
        infos_state = INFOS_SHOWING;
        animation_start_time = SDL_GetTicks();
    }
}

// Démarre l'animation de disparition
void infos_display_hide_animation(AppContext* context) {
    if (infos_state != INFOS_HIDDEN && infos_state != INFOS_HIDING) {
        infos_state = INFOS_HIDING;
        animation_start_time = SDL_GetTicks();
    }
}

/* Convertit des coordonnées bandeau (même repère que display_image/text_display)
   en coordonnées écran absolues pour un rectangle de taille (w, h). */
static int bandeau_to_screen_x(int bx, int w) {
    return (WIN_WIDTH - w) / 2 + bx;
}
static int bandeau_to_screen_y(int by, int h) {
    return (WIN_HEIGHT - h) / 2 - by;
}

static void window_to_logical(AppContext* context, int wx, int wy, int* lx, int* ly) {
    if (!lx || !ly) return;

    if (!context || !context->renderer) {
        *lx = wx;
        *ly = wy;
        return;
    }

    float logical_x = (float)wx;
    float logical_y = (float)wy;
    SDL_RenderWindowToLogical(context->renderer, wx, wy, &logical_x, &logical_y);
    *lx = (int)lroundf(logical_x);
    *ly = (int)lroundf(logical_y);
}

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

/* ratio=0 -> rouge, ratio=1 -> vert */
static SDL_Color color_red_to_green(float ratio) {
    float t = clamp01(ratio);
    Uint8 r = (Uint8)(255.0f * (1.0f - t));
    Uint8 g = (Uint8)(255.0f * t);
    return (SDL_Color){r, g, 0, 255};
}

static SDL_Color color_for_fps(float fps) {
    const float fps_min = 30.0f;
    const float fps_max = 60.0f;
    float ratio = (fps - fps_min) / (fps_max - fps_min);
    return color_red_to_green(ratio);
}

static SDL_Color color_for_ping(int ping_ms) {
    const float ping_good = 10.0f;
    const float ping_bad = 150.0f;
    float bad_ratio = ((float)ping_ms - ping_good) / (ping_bad - ping_good);
    return color_red_to_green(1.0f - bad_ratio);
}

/* Met à jour la position des crossfaders pour qu'ils suivent le bandeau */
static void update_crossfader_positions(int display_x) {
    /* Même décalage X que le texte FPS */
    int bx = display_x + 50;

    if (cf_music && cf_music->cfg) {
        int sx = bandeau_to_screen_x(bx, cf_music->cfg->w);
        int sy = bandeau_to_screen_y(320, cf_music->cfg->h);  /* en dessous du ping */
        cf_music->cfg->x = sx;
        cf_music->cfg->y = sy;
        cf_music->cfg->rect.x = sx;
        cf_music->cfg->rect.y = sy;
        cf_music->cfg->hidden = (infos_state == INFOS_HIDDEN);
    }
    if (cf_sfx && cf_sfx->cfg) {
        int sx = bandeau_to_screen_x(bx, cf_sfx->cfg->w);
        int sy = bandeau_to_screen_y(250, cf_sfx->cfg->h);  /* encore en dessous */
        cf_sfx->cfg->x = sx;
        cf_sfx->cfg->y = sy;
        cf_sfx->cfg->rect.x = sx;
        cf_sfx->cfg->rect.y = sy;
        cf_sfx->cfg->hidden = (infos_state == INFOS_HIDDEN);
    }
}

// Affiche les informations à l'écran
void infos_display(AppContext* context) {
    if (bandeau) {
        // Récupérer la position de la souris
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        int logical_mouse_x = mouse_x;
        int logical_mouse_y = mouse_y;
        window_to_logical(context, mouse_x, mouse_y, &logical_mouse_x, &logical_mouse_y);
        
        // Constantes pour le positionnement
        const int TRIGGER_OPEN_X = 20; // Seuil X pour déclencher l'animation
        const int TRIGGER_CLOSE_X = 240; // Seuil X pour refermer l'animation
        const int BASE_X = -900; // Position visible du bandeau
        const int HIDDEN_X = -1300; // Position hors écran (caché)

        // Déterminer l'état actuel et les positions d'animation
        Uint32 current_time = SDL_GetTicks();
        float progress = 0.0f;
        int display_x = HIDDEN_X;
        
        // Gérer les transitions d'état basées sur la souris
        if (infos_state == INFOS_HIDDEN && mouse_x < TRIGGER_OPEN_X) {
            // La souris vient d'entrer : déclencher l'apparition
            infos_state = INFOS_SHOWING;
            animation_start_time = SDL_GetTicks();
        } 
        else if (infos_state == INFOS_VISIBLE && mouse_x > TRIGGER_CLOSE_X) {
            // La souris vient de sortir : déclencher la disparition
            infos_state = INFOS_HIDING;
            animation_start_time = SDL_GetTicks();
        }
        
        // Gérer les animations en cours
        if (infos_state == INFOS_SHOWING) {
            progress = (float)(current_time - animation_start_time) / ANIMATION_DURATION;
            if (progress >= 1.0f) {
                progress = 1.0f;
                infos_state = INFOS_VISIBLE;
            }
            // Interpolation du sliding en apparition : HIDDEN_X -> BASE_X
            display_x = HIDDEN_X + (BASE_X - HIDDEN_X) * progress;
        } 
        else if (infos_state == INFOS_HIDING) {
            progress = (float)(current_time - animation_start_time) / ANIMATION_DURATION;
            if (progress >= 1.0f) {
                progress = 1.0f;
                infos_state = INFOS_HIDDEN;
            }
            // Interpolation du sliding en disparition : BASE_X -> HIDDEN_X
            display_x = BASE_X + (HIDDEN_X - BASE_X) * progress;
        } 
        else if (infos_state == INFOS_VISIBLE) {
            // Reste visible à la position finale
            display_x = BASE_X;
        }
        // Si INFOS_HIDDEN, display_x reste HIDDEN_X

        current_display_x = display_x;
        update_crossfader_positions(display_x);
        
        // N'afficher que si pas complètement caché
        if (infos_state != INFOS_HIDDEN) {
            display_image(context->renderer, bandeau, display_x, 0, 0.95, 90, SDL_FLIP_NONE, 1, 255);
            fps_ping_display(context, display_x);

            /* Labels des crossfaders - mise à jour position et affichage */
            int label_x = display_x + 50;
            
            update_text_position(txt_music_label, label_x, 345);
            display_text(context, txt_music_label);
            
            update_text_position(txt_sfx_label, label_x, 275);
            display_text(context, txt_sfx_label);

            /* Affichage des règles du jeu */
            int rules_start_y = 180;
            int line_spacing = 22;

            // Titre des règles
            update_text_position(txt_rules_title, label_x, rules_start_y);
            display_text(context, txt_rules_title);
            
            // Lignes des règles
            for (int i = 0; i < rules_line_count; i++) {
                if (txt_rules_lines && txt_rules_lines[i]) {
                    update_text_position(txt_rules_lines[i], label_x, rules_start_y - 35 - (i * line_spacing));
                    display_text(context, txt_rules_lines[i]);
                }
            }

            /* Affichage de la version du jeu */
            if (txt_version) {
                update_text_position(txt_version, label_x, -450);
                display_text(context, txt_version);
            }

            /* Rendu des crossfaders */
            crossfaders_render(context->renderer);
        }
    }
}

void infos_handle_event(AppContext* context, SDL_Event* event) {
    crossfaders_handle_event(context, event);
}

// FPS calculation
void calculate_fps(AppContext* context, Uint32 current_time) {
    static Uint32 last_time = 0;
    static int frame_count = 0;
    
    if (last_time == 0) {
        last_time = current_time;
    }

    frame_count++;
    if (current_time - last_time >= 100) { // Mettre à jour les FPS toutes les 100 ms
        context->fps = (frame_count * 1000.0f) / (current_time - last_time);
        frame_count = 0;
        last_time = current_time;
    }
}


// FPS display
void fps_ping_display(AppContext* context, int display_x) {
    calculate_fps(context, SDL_GetTicks());
    context->ping_ms = get_tcp_ping_ms(context->sock);

    int base_x = display_x + 50;

    // Affichage des FPS qui suit le bandeau
    char fps_text_buf[20];
    snprintf(fps_text_buf, sizeof(fps_text_buf), "FPS : %.2f", context->fps);
    SDL_Color fps_color = color_for_fps(context->fps);
    
    update_text(context, txt_fps, fps_text_buf);
    update_text_color(context, txt_fps, fps_color);
    update_text_position(txt_fps, base_x, 450);
    display_text(context, txt_fps);

    // Affichage du ping
    char ping_text_buf[32];
    if (context->ping_ms >= 0) {
        snprintf(ping_text_buf, sizeof(ping_text_buf), "Ping : %d ms", context->ping_ms);
    } else {
        snprintf(ping_text_buf, sizeof(ping_text_buf), "Ping : --");
    }

    SDL_Color ping_color = (context->ping_ms >= 0)
        ? color_for_ping(context->ping_ms)
        : COL_WHITE;

    update_text(context, txt_ping, ping_text_buf);
    update_text_color(context, txt_ping, ping_color);
    update_text_position(txt_ping, base_x, 410);
    display_text(context, txt_ping);

    /* Info sur le type de serveur (local/distant) */
    if (is_tcp_local_server(context->sock)) {
        update_text_position(txt_server_local, base_x, 385);
        display_text(context, txt_server_local);
    } else if (context->sock >= 0) {
        update_text_color(context, txt_server_distant, ping_color);
        update_text_position(txt_server_distant, base_x, 385);
        display_text(context, txt_server_distant);
    }

    /* Barres horizontales décoratives */
    update_text_position(txt_bar_1, base_x, 370);
    display_text(context, txt_bar_1);
    
    update_text_position(txt_bar_2, base_x, 230);
    display_text(context, txt_bar_2);
}

int infos_free() {
    if (bandeau) {
        SDL_DestroyTexture(bandeau);
        bandeau = NULL;
    }
    crossfaders_free();
    free_rules();
    
    /* Libération des textes optimisés */
    destroy_text(txt_music_label); txt_music_label = NULL;
    destroy_text(txt_sfx_label); txt_sfx_label = NULL;
    destroy_text(txt_rules_title); txt_rules_title = NULL;
    
    if (txt_rules_lines) {
        for (int i = 0; i < rules_line_count; i++) {
            destroy_text(txt_rules_lines[i]);
        }
        free(txt_rules_lines);
        txt_rules_lines = NULL;
    }
    
    destroy_text(txt_fps); txt_fps = NULL;
    destroy_text(txt_ping); txt_ping = NULL;
    destroy_text(txt_server_local); txt_server_local = NULL;
    destroy_text(txt_server_distant); txt_server_distant = NULL;
    destroy_text(txt_bar_1); txt_bar_1 = NULL;
    destroy_text(txt_bar_2); txt_bar_2 = NULL;
    destroy_text(txt_version); txt_version = NULL;
    
    return EXIT_SUCCESS;
}