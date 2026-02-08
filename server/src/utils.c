#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "../lib/utils.h"

int randint(int min, int max) {
    // Retourne un entier aléatoire entre min et max inclus
    // Formule : min + rand() / (RAND_MAX / (max - min + 1) + 1)
    int range = max - min + 1;
    return min + rand() / (RAND_MAX / range + 1);
}

int count_words(const char *filepath) {
    if (!filepath) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    char buf[4096];
    int count = 0;

    while (fgets(buf, sizeof(buf), f) != NULL) {
        /* vérifier s'il y a au moins un caractère non blanc dans la ligne */
        int has_non_ws = 0;
        for (size_t i = 0; buf[i] != '\0'; ++i) {
            if (!isspace((unsigned char)buf[i])) {
                has_non_ws = 1;
                break;
            }
        }
        if (has_non_ws) count++;

        /* consommer la suite d'une ligne trop longue pour le tampon */
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n') {
            int c;
            do {
                c = fgetc(f);
            } while (c != '\n' && c != EOF);
        }
    }

    fclose(f);
    return count;
}

int starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    size_t len_prefix = strlen(prefix);
    return strncmp(str, prefix, len_prefix) == 0;
}

int number_length(int n) {
    if (n == 0) return 1;
    int length = 0;
    if (n < 0) {
        length++; // signe negatif a ajouter
        n = -n; // rendre n positif pour le calcul de la longueur des chiffres
    }
    while (n > 0) {
        n /= 10;
        length++;
    }
    return length;
}