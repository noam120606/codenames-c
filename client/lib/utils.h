#ifndef UTILS_H
#define UTILS_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

// Aliases divers
static const SDL_Color COL_WHITE = (SDL_Color){255, 255, 255, 255};
static const SDL_Color COL_BLACK = (SDL_Color){0, 0, 0, 255};
static const SDL_Color COL_RED = (SDL_Color){255, 0, 0, 255};
static const SDL_Color COL_BLUE = (SDL_Color){0, 0, 255, 255};
static const SDL_Color COL_GREEN = (SDL_Color){0, 255, 0, 255};
static const SDL_Color COL_YELLOW = (SDL_Color){255, 255, 0, 255};
static const SDL_Color COL_ORANGE = (SDL_Color){255, 165, 0, 255};
static const SDL_Color COL_PURPLE = (SDL_Color){128, 0, 128, 255};
static const SDL_Color COL_CYAN = (SDL_Color){0, 255, 255, 255};
static const SDL_Color COL_MAGENTA = (SDL_Color){255, 0, 255, 255};

#define FONT_LARABIE "assets/fonts/larabiefont.otf"




/**
 * Calcule la longueur d'un entier en nombre de chiffres.
 */
int number_length(int n);

/**
 * Formate dans un buffer fourni sans allocation dynamique.
 * @param buf Buffer de sortie.
 * @param size Taille du buffer (en octets).
 * @param fmt Chaîne de format (printf-like).
 * @return Nombre de caractères écrits (sans le '\0'), ou -1 en cas d'erreur.
 */
int format_to(char *buf, size_t size, const char *fmt, ...);

/**
 * Convertit un entier en chaîne de caractères.
 * @param value Entier à convertir.
 * @return Chaîne de caractères représentant l'entier.
 */
char* itoa(int value);

/**
 * Retourne un nom d'utilisateur aléatoire (alloué, doit être free()).
 * Lit le fichier `assets/misc/usernames.txt`.
 */
char* getRandomUsername(void);

#endif // UTILS_H