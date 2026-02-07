#ifndef UTILS_H
#define UTILS_H

/** Génère un entier aléatoire entre min et max inclus.
 * @param min Valeur minimale.
 * @param max Valeur maximale.
 * @return Valeur aléatoire comprise entre min et max.
 */
int randint(int min, int max);

/** Retourne le nombre de mots (lignes non vides) dans le fichier filepath.
 * @param filepath Chemin du fichier à analyser.
 * @return Nombre de lignes non vides, ou -1 en cas d'erreur.
 */
int count_words(const char *filepath);

#endif // UTILS_H
