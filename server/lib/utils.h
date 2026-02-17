#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/** 
 * Génère un entier aléatoire entre min et max inclus.
 * @param min Valeur minimale.
 * @param max Valeur maximale.
 * @return Valeur aléatoire comprise entre min et max.
 */
int randint(int min, int max);

/** 
 * Retourne le nombre de mots (lignes non vides) dans le fichier filepath.
 * @param filepath Chemin du fichier à analyser.
 * @return Nombre de lignes non vides, ou -1 en cas d'erreur.
 */
int count_words(const char *filepath);

/** 
 * Vérifie si la chaîne str commence par le préfixe prefix.
 * @param str Chaîne à vérifier.
 * @param prefix Préfixe à rechercher.
 * @return 1 si str commence par prefix, 0 sinon.
 */
int starts_with(const char *str, const char *prefix);

/** 
 * Calcule le nombre de chiffres nécessaires pour représenter un entier n en base 10.
 * @param n Entier à analyser.
 * @return Nombre de chiffres nécessaires pour représenter n, ou 1 si n est 0.
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


#endif // UTILS_H
