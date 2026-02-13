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
 * @param angle Angle de rotation du texte en degrés (0 par défaut, sens horaire)
 * @param opacity Opacité du texte (0-255, 255 par défaut)
 * Note : Le texte est affiché centré en (x, y).
 */
void text_display(SDL_Renderer* renderer, const char* text, const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity);

#endif /* TEXT_H */