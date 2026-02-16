#include "../lib/all.h"

SDL_Texture* bandeau;

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

int init_infos(SDL_Context* context) {
    int loading_fails = 0;

    bandeau = load_image(context->renderer, "assets/img/others/bandeau_infos.png");
    if (!bandeau) {
        printf("Failed to load bandeau image\n");
        loading_fails++;
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

// Affiche les informations à l'écran
void infos_display(SDL_Context* context) {
    if (bandeau) {
        // Récupérer la position de la souris
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        // Constantes pour le positionnement
        const int TRIGGER_X = 200; // Seuil X pour déclencher l'animation
        const int BASE_X = -950; // Position visible du bandeau
        const int HIDDEN_X = -1300; // Position hors écran (caché)
        
        // Vérifier si la souris est dedans (au-dessus du seuil)
        int mouse_inside = (mouse_x <= TRIGGER_X);
        
        // Déterminer l'état actuel et les positions d'animation
        Uint32 current_time = SDL_GetTicks();
        float progress = 0.0f;
        int display_x = HIDDEN_X;
        int start_x, end_x;
        
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
        
        // N'afficher que si pas complètement caché
        if (infos_state != INFOS_HIDDEN) {
            display_image(context->renderer, bandeau, display_x, 0, 0.75, 90, SDL_FLIP_NONE, 1, 192);
            fps_display(context, display_x);
        }
    }
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
void fps_display(SDL_Context* context, int display_x) {
    calculate_fps(context, SDL_GetTicks());
    // Affichage des FPS qui suit le bandeau
    char fps_text[20];
    snprintf(fps_text, sizeof(fps_text), "FPS : %.2f", context->fps);
    SDL_Color white = {255, 255, 255, 255};
    // Positionner le texte relativement au display_x du bandeau
    int fps_x = display_x + 100;
    int fps_y = 450;
    text_display(context->renderer, fps_text, "assets/fonts/larabiefont.otf", 22, white, fps_x, fps_y, 0, 255);
}

void infos_free() {
    if (bandeau) {
        SDL_DestroyTexture(bandeau);
        bandeau = NULL;
    }
}