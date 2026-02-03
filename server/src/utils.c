#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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


