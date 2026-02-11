#include "../lib/all.h"

SDL_Texture* lens;

int init_background(SDL_Context * context) {
    int loading_fails = 0;

    // Force nearest neighbor scaling (préserver les détails lors de la réduction)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"); // "0" = nearest, "1" = linear, "2" = best
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
    
    // Espacements proportionnels à la taille des symboles
    const float SPACE_X_RATIO = 0.5f;   // 50% de la largeur
    const float SPACE_Y_RATIO = 1.0f;   // 100% de la hauteur

    const int SPACE_X = (int)(SYMBOL_W * SPACE_X_RATIO);
    const int SPACE_Y = (int)(SYMBOL_H * SPACE_Y_RATIO);

    const float speed = 0.60;

    const int TILE_W = SYMBOL_W + SPACE_X;
    const int TILE_H = SYMBOL_H + SPACE_Y;

    const int EXTRA_TILES = 4;

    const int cols = WIN_WIDTH  / SYMBOL_W + EXTRA_TILES;
    const int rows = WIN_HEIGHT / SYMBOL_H + EXTRA_TILES;

    // Temps ajusté par la vitesse
    const int scaled_time = (int)(context->clock * speed);

    const int time_x = scaled_time % (4 * TILE_W);
    const int time_y = scaled_time % (2 * TILE_H);

    const int offset_x = (time_x - SPACE_X * 2) / 2;

    for (int i = -EXTRA_TILES; i < cols; i++)    {
        const int base_x = i * TILE_W - offset_x;
        const int row_offset = (i % 2) ? SPACE_Y : 0;

        for (int j = -EXTRA_TILES; j < rows; j++){
            const int x = base_x;
            const int y = j * TILE_H + time_y + row_offset;

            SDL_RenderCopy(context->renderer, lens, NULL, &(SDL_Rect){x, y, SYMBOL_W, SYMBOL_H});
            //display_image(context->renderer, lens, x, y, SYMBOL_W, SYMBOL_H);
        }
    }
}


void destroy_background() {
    if (lens) free_image(lens);
}