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

/**
 * Calcule la distance de Levenshtein entre deux chaînes (insensible à la casse)
 * @param s1 Première chaîne
 * @param s2 Deuxième chaîne
 * @return Distance de Levenshtein (0 = identique, plus c'est grand plus c'est différent)
 */
int levenshtein(const char* s1, const char* s2);

/**
 * Vérifie si un mot est trop proche d'un mot de carte (à bannir)
 * @param input Le mot entré par le joueur
 * @param card_word Le mot sur la carte
 * @param threshold Seuil de distance max (ex: 2 = modifications max autorisées)
 * @return 1 si le mot est trop proche (à bannir), 0 sinon
 */
int is_word_too_close(const char* input, const char* card_word, int threshold);

/**
 * Vérifie si un mot contient un autre mot (sous-chaîne, insensible à la casse)
 * @param input Le mot entré
 * @param card_word Le mot de la carte
 * @return 1 si input contient card_word ou vice versa, 0 sinon
 */
int word_contains(const char* input, const char* card_word);

#endif // UTILS_H