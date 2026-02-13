#include "../lib/all.h"

void text_display(SDL_Renderer* renderer, const char* text, const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity) {
    TTF_Font* font = TTF_OpenFont(font_path, size);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }
    int tex_w = 0, tex_h = 0;

    /* Point de pivot pour la rotation */
    SDL_Point center = { tex_w / 2, tex_h / 2 };

    /* Appliquer l'opacité à la texture du texte */
    SDL_SetTextureAlphaMod(texture, opacity);

    /* Appliquer l'opacité */ 
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);

    // Centrer le texte dans la fenêtre
    int final_x = (WIN_WIDTH - tex_w) / 2 + x;
    int final_y = (WIN_HEIGHT - tex_h) / 2 - y; /* Inversion de l'axe y (seulement)*/

    SDL_Rect dst_rect = { final_x, final_y, tex_w, tex_h };
    SDL_RenderCopy(renderer, texture, NULL, &dst_rect);

    // Nettoyage
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}