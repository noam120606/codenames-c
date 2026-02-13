#ifndef TEXT_H
#define TEXT_H

#include "sdl.h"

/**
 * Affiche du texte à l'écran.
 * @param renderer Le renderer SDL
 * @param text Le texte à afficher
 * @param font_path Chemin vers la police à utiliser
 * @param size Taille de la police
 * @param color Couleur du texte (SDL_Color)
 * @param x Position x du centre du texte (`0` centre x de la fenêtre)
 * @param y Position y du centre du texte (`0` centre y de la fenêtre)
 * Note : Le texte est affiché centré en (x, y).
 */
void text_display(SDL_Renderer* renderer, const char* text, const char* font_path, int size, SDL_Color color, int x, int y);

#endif /* TEXT_H */