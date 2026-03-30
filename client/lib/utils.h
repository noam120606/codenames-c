/**
 * @file utils.h
 * @brief Fonctions utilitaires diverses (formatage, couleurs, chaînes).
 */

#ifndef UTILS_H
#define UTILS_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/game.h"

// Aliases divers
static const SDL_Color COL_WHITE = (SDL_Color){255, 255, 255, 255};
static const SDL_Color COL_BLACK = (SDL_Color){0, 0, 0, 255};
static const SDL_Color COL_LIGHT_GRAY = (SDL_Color){200, 200, 200, 255};
static const SDL_Color COL_GRAY = (SDL_Color){128, 128, 128, 255};
static const SDL_Color COL_DARK_GRAY = (SDL_Color){50, 50, 50, 255};
static const SDL_Color COL_RED = (SDL_Color){255, 0, 0, 255};
static const SDL_Color COL_BLUE = (SDL_Color){0, 0, 255, 255};
static const SDL_Color COL_LIGHT_GREEN = (SDL_Color){144, 238, 144, 255};
static const SDL_Color COL_GREEN = (SDL_Color){0, 255, 0, 255};
static const SDL_Color COL_DARK_GREEN = (SDL_Color){0, 128, 0, 255};
static const SDL_Color COL_YELLOW = (SDL_Color){255, 255, 0, 255};
static const SDL_Color COL_ORANGE = (SDL_Color){255, 165, 0, 255};
static const SDL_Color COL_PURPLE = (SDL_Color){128, 0, 128, 255};
static const SDL_Color COL_CYAN = (SDL_Color){0, 255, 255, 255};
static const SDL_Color COL_MAGENTA = (SDL_Color){255, 0, 255, 255};

#define FONT_LARABIE "assets/fonts/larabiefont.otf"
#define FONT_NOTO "assets/fonts/NotoSansSymbols.ttf"




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

#ifndef _WIN32
/**
 * Convertit un entier en chaîne (API historique Linux).
 * Retourne un buffer statique (non thread-safe).
 */
char* itoa(int value);
#endif

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
 * Vérifie si un mot contient un autre mot (sous-chaîne, insensible à la casse)
 * @param input Le mot entré
 * @param card_word Le mot de la carte
 * @return 1 si input contient card_word ou vice versa, 0 sinon
 */
int word_contains(const char* input, const char* card_word);

/**
 * Vérifie si un indice de mot est valide par rapport à une liste de mots de carte
 * @param hint Le mot à vérifier
 * @param card_words La liste des mots de carte
 * @return 1 si le mot est valide, 0 sinon
 */
int valid_hint(const char* hint, Word card_words[NB_WORDS]);

#endif // UTILS_H