#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int randint(int min, int max) {
    // Retourne un entier aléatoire entre min et max inclus
    // Formule : min + rand() / (RAND_MAX / (max - min + 1) + 1)
    int range = max - min + 1;
    return min + rand() / (RAND_MAX / range + 1);
}

/*Retourne le nombre de mots (lignes non vides) dans le fichier filepath 
et retourne -1 en cas d'erreur (par exemple si le fichier filepath est introuvable ...)*/
int count_words(const char *filepath);
