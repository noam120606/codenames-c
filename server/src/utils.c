#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int randint(int min, int max) {
    // Retourne un entier aléatoire entre min et max inclus
    // Formule : min + rand() / (RAND_MAX / (max - min + 1) + 1)
    int range = max - min + 1;
    return min + rand() / (RAND_MAX / range + 1);
}