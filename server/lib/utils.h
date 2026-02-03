#ifndef UTILS_H
#define UTILS_H

int randint(int min, int max);

/*Retourne le nombre de mots (lignes non vides) dans le fichier filepath 
et retourne -1 en cas d'erreur (par exemple si le fichier filepath est introuvable ...)*/
int count_words(const char *filepath);

#endif // UTILS_H
