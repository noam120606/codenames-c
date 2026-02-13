#include "../lib/all.h"

SDL_Texture* placeholder;

int init_infos(SDL_Context* context) {
    int loading_fails = 0;

    placeholder = load_image(context->renderer, "assets/img/others/placeholder_infos.png");
    if (!placeholder) {
        printf("Failed to load placeholder image\n");
        loading_fails++;
    }
    return loading_fails;
}
// Affiche les informations à l'écran
void infos_display(SDL_Context* context) {
    if (placeholder) {
        // Récupérer la position de la souris
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        
        // Constantes pour l'effet de révélation
        const int REVEAL_DISTANCE = 200; // Distance du bord gauche pour commencer la révélation
        const int BASE_X = -1700; // Position nominale du placeholder
        const int HIDDEN_X = -1950; // Position hors écran (caché)
        
        // Calculer le facteur de révélation (0 = caché, 1 = visible)
        float reveal_factor = 1.0f - ((float)mouse_x / REVEAL_DISTANCE);
        
        // Limiter entre 0 et 1
        if (reveal_factor < 0.0f) reveal_factor = 0.0f;
        if (reveal_factor > 1.0f) reveal_factor = 1.0f;
        
        // Calculer la position x avec interpolation
        int display_x = HIDDEN_X + (BASE_X - HIDDEN_X) * reveal_factor;
        
        display_image(context->renderer, placeholder, display_x, 0, 1, 0, SDL_FLIP_NONE, 1, 64);
        fps_display(context, display_x);
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
    // Affichage des FPS qui suit le placeholder
    char fps_text[20];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", context->fps);
    SDL_Color white = {255, 255, 255, 255};
    // Positionner le texte relativement au display_x du placeholder
    int fps_x = display_x + 850;
    int fps_y = 525;
    text_display(context->renderer, fps_text, "assets/fonts/larabiefont.otf", 18, white, fps_x, fps_y, 0, 255);
}

void infos_free() {
    if (placeholder) {
        SDL_DestroyTexture(placeholder);
        placeholder = NULL;
    }
}