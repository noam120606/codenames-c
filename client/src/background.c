#include "../lib/all.h"

SDL_Texture* lens;

int init_background(SDL_Context * context) {
    int loading_fails = 0;

    // Force nearest neighbor scaling (préserver les détails lors de la réduction)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // "0" = nearest, "1" = linear, "2" = best
    lens = load_image(context->renderer, "assets/img/background/lens.png");
    if (!lens) {
        printf("Failed to load lens image\n");
        loading_fails++;
    }

    // Utiliser nearest neighbor pour éviter le lissage lors du rendu à plus petite taille
    SDL_SetTextureScaleMode(lens, SDL_ScaleModeNearest);

    return loading_fails;
}

void display_background(SDL_Context* context){
    const int SYMBOL_W = 64;
    const int SYMBOL_H = 64;
    const int SYMBOL_SPACING = 32; // Espacement entre les symboles
    const int EXTRA_TILES = 15; // Nombre de tuiles supplémentaires pour couvrir les bords lors du déplacement
    const float speed = 0.55; // Vitesse de déplacement du fond
    const float size = 0.12;
    
    // Constantes pour l'effet de grossissement
    const int INTERACT_RADIUS = 150; // Rayon d'interaction à la souris
    const float MAX_SCALE = 1.5f; // Maxi multiplicateur de taille

    const int TILE_W = SYMBOL_W + SYMBOL_SPACING;
    const int TILE_H = SYMBOL_H + SYMBOL_SPACING;

    const int cols = (WIN_WIDTH / TILE_W) + EXTRA_TILES;
    const int rows = (WIN_HEIGHT / TILE_H) + EXTRA_TILES;

    // Temps ajusté par la vitesse
    const int scaled_time = (int)(context->clock * speed);

    // Décalage horizontal et vertical en pixels
    const int offset_x = (scaled_time) % (TILE_W * 2);
    const int offset_y = (scaled_time) % (TILE_H * 2);
    
    // Obtenir la position de la souris
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    // Correction de la position de la souris pour correspondre au système de coordonnées centré
    mouse_x -= WIN_WIDTH / 2;
    mouse_y = (WIN_HEIGHT / 2) - mouse_y; // Inversion de l'axe y pour correspondre au système de coordonnées centré

    for (int i = -EXTRA_TILES; i < cols; i++) {
        // Décalage vertical pour créer un effet de quinconces
        const int row_offset = (i % 2) ? TILE_H / 2 : 0;
        
        for (int j = -EXTRA_TILES; j < rows; j++) {
            const int x = (i * TILE_W) - offset_x;
            const int y = (j * TILE_H) - offset_y + row_offset;
            
            // Calculer la distance avec la souris
            const int dx = x + (int)(SYMBOL_W * size / 2) - mouse_x;
            const int dy = y + (int)(SYMBOL_H * size / 2) - mouse_y;
            const int distance = (int)sqrt(dx * dx + dy * dy);
            
            // Calculer le multiplicateur de taille en fonction de la distance
            float scale = 1.0f;
            if (distance < INTERACT_RADIUS) {
                // Plus on est proche, plus c'est gros
                scale = 1.0f + (1.0f - (float)distance / INTERACT_RADIUS) * (MAX_SCALE - 1.0f);
            }

            display_image(context->renderer, lens, x, y, size * scale, 0, row_offset == 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 1, 255);
        }
    }
}

void destroy_background() {
    if (lens) free_image(lens);
}