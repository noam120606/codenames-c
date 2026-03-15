#include "../lib/all.h"

SDL_Texture* bandeau;

// Crossfaders pour le volume
static Crossfader* cf_music = NULL;
static Crossfader* cf_sfx = NULL;

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
static int mouse_was_inside = 0; // Indique si la souris était dans la zone à la dernière vérification

// Position courante du bandeau (mise à jour chaque frame)
static int current_display_x = -1300;

/* Callback appelé quand le crossfader musique change */
static void on_music_volume_change(SDL_Context* ctx, int new_value) {
    if (ctx) ctx->music_volume = new_value;
    audio_set_type_volume(AUDIO_SOUND_KIND_MUSIC, new_value);
}

/* Callback appelé quand le crossfader effets sonores change */
static void on_sfx_volume_change(SDL_Context* ctx, int new_value) {
    if (ctx) ctx->sound_effects_volume = new_value;
    audio_set_type_volume(AUDIO_SOUND_KIND_SFX, new_value);
}

int init_infos(SDL_Context* context) {
    int loading_fails = 0;

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

    return loading_fails;
}

// Démarre l'animation d'apparition
void infos_display_show_animation(SDL_Context* context) {
    if (infos_state != INFOS_VISIBLE && infos_state != INFOS_SHOWING) {
        infos_state = INFOS_SHOWING;
        animation_start_time = SDL_GetTicks();
    }
}

// Démarre l'animation de disparition
void infos_display_hide_animation(SDL_Context* context) {
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

static void window_to_logical(SDL_Context* context, int wx, int wy, int* lx, int* ly) {
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
    int bx = display_x + 100;

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
void infos_display(SDL_Context* context) {
    if (bandeau) {
        // Récupérer la position de la souris
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        int logical_mouse_x = mouse_x;
        int logical_mouse_y = mouse_y;
        window_to_logical(context, mouse_x, mouse_y, &logical_mouse_x, &logical_mouse_y);
        
        // Constantes pour le positionnement
        const int TRIGGER_X = 300; // Seuil X pour déclencher l'animation
        const int BASE_X = -950; // Position visible du bandeau
        const int HIDDEN_X = -1300; // Position hors écran (caché)
        
        // Vérifier si la souris est dedans (au-dessus du seuil)
        int mouse_inside = (logical_mouse_x <= TRIGGER_X);
        
        // Déterminer l'état actuel et les positions d'animation
        Uint32 current_time = SDL_GetTicks();
        float progress = 0.0f;
        int display_x = HIDDEN_X;
        
        // Gérer les transitions d'état basées sur la souris
        if (mouse_inside && !mouse_was_inside) {
            // La souris vient d'entrer : déclencher l'apparition
            infos_state = INFOS_SHOWING;
            animation_start_time = SDL_GetTicks();
            mouse_was_inside = 1;
        } 
        else if (!mouse_inside && mouse_was_inside) {
            // La souris vient de sortir : déclencher la disparition
            infos_state = INFOS_HIDING;
            animation_start_time = SDL_GetTicks();
            mouse_was_inside = 0;
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
            display_image(context->renderer, bandeau, display_x, 0, 0.75, 90, SDL_FLIP_NONE, 1, 192);
            fps_ping_display(context, display_x);

            /* Labels des crossfaders (coordonnées bandeau, même repère que text_display) */
            int label_x = display_x + 100;
            text_display(context->renderer, "Musique", FONT_LARABIE, 18, COL_WHITE, label_x, 345, 0, 255);
            text_display(context->renderer, "Effets Sonores", FONT_LARABIE, 18, COL_WHITE, label_x, 275, 0, 255);

            /* Rendu des crossfaders */
            crossfaders_render(context->renderer);
        }
    }
}

void infos_handle_event(SDL_Context* context, SDL_Event* event) {
    crossfaders_handle_event(context, event);
}

// FPS calculation
void calculate_fps(SDL_Context* context, Uint32 current_time) {
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
void fps_ping_display(SDL_Context* context, int display_x) {
    calculate_fps(context, SDL_GetTicks());
    context->ping_ms = get_tcp_ping_ms(context->sock);

    // Affichage des FPS qui suit le bandeau
    char fps_text[20];
    snprintf(fps_text, sizeof(fps_text), "FPS : %.2f", context->fps);
    SDL_Color fps_color = color_for_fps(context->fps);
    // Positionner le texte relativement au display_x du bandeau
    int fps_x = display_x + 100;
    int fps_y = 450;
    text_display(context->renderer, fps_text, "assets/fonts/larabiefont.otf", 22, fps_color, fps_x, fps_y, 0, 255);

    char ping_text[32];
    if (context->ping_ms >= 0) {
        snprintf(ping_text, sizeof(ping_text), "Ping : %d ms", context->ping_ms);
    } else {
        snprintf(ping_text, sizeof(ping_text), "Ping : --");
    }

    SDL_Color ping_color = (context->ping_ms >= 0)
        ? color_for_ping(context->ping_ms)
        : COL_WHITE;

    int ping_x = display_x + 100;
    int ping_y = 410;
    text_display(context->renderer, ping_text, "assets/fonts/larabiefont.otf", 22, ping_color, ping_x, ping_y, 0, 255);

    /* Info sur le type de serveur (local/distant) à côté du ping */
    if (is_tcp_local_server(context->sock)) {
        int local_x = display_x + 100;
        int local_y = 385;
        text_display(context->renderer, "(serveur local)", "assets/fonts/larabiefont.otf", 14, COL_GREEN, local_x, local_y, 0, 220);
    } else if (context->sock >= 0) {
        int local_x = display_x + 100;
        int local_y = 385;
        /* La couleur est la même que celle du ping si le serveur est distant */
        text_display(context->renderer, "(serveur distant)", "assets/fonts/larabiefont.otf", 14, ping_color, local_x, local_y, 0, 220);
    }

    /* Barre horizontale décorative sous les fps et ping */
    char horizontal_bar_text[32];
    snprintf(horizontal_bar_text, sizeof(horizontal_bar_text), "______________");
    int horizontal_bar_x = display_x + 100;
    int horizontal_bar_y = 370;
    text_display(context->renderer, horizontal_bar_text, "assets/fonts/larabiefont.otf", 22, COL_WHITE, horizontal_bar_x, horizontal_bar_y, 0, 255);
}

int infos_free() {
    if (bandeau) {
        SDL_DestroyTexture(bandeau);
        bandeau = NULL;
    }
    crossfaders_free();
    return EXIT_SUCCESS;
}