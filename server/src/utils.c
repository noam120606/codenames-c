#include "../lib/all.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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

int format_to(char *buf, size_t size, const char *fmt, ...) {
    if (!buf || size == 0 || !fmt) return -1;
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buf, size, fmt, args);
    va_end(args);
    if (ret < 0) return -1;
    return ret;
}

char* generate_code() {
    char buffer[6];
    for (int i = 0; i < 5; i++) {
        buffer[i] = '0' + rand() % 10;
    }
    buffer[5] = '\0';

    return strdup(buffer);
}

/**
 * Génère une chaîne UUID v4-like aléatoire (format 8-4-4-4-12).
 */
static void random_uuid_string(char* out, size_t size) {
    const char hex[] = "0123456789abcdef";
    // Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx  (36 chars + '\0')
    if (size < 37) return;
    int pos = 0;
    int lengths[] = {8, 4, 4, 4, 12};
    for (int g = 0; g < 5; g++) {
        if (g > 0) out[pos++] = '-';
        for (int i = 0; i < lengths[g]; i++) {
            out[pos++] = hex[rand() % 16];
        }
    }
    out[pos] = '\0';
}

/**
 * Vérifie si un UUID existe déjà dans le fichier.
 */
static int uuid_exists_in_file(const char* filepath, const char* uuid) {
    FILE* f = fopen(filepath, "r");
    if (!f) return 0; // fichier n'existe pas => pas de doublon
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        // Supprimer le '\n' en fin de ligne
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strcmp(line, uuid) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

char* generate_uuid(const char* uuids_path) {
    if (!uuids_path) return NULL;

    // S'assurer que le répertoire parent (data) existe
    {
        struct stat st = {0};
        if (stat("data", &st) == -1) {
            if (mkdir("data", 0755) != 0) {
                perror("Warning: could not create 'data' directory");
            }
        }
    }

    // Générer un UUID unique
    char uuid[37];
    int max_attempts = 1000;
    int found = 0;
    for (int i = 0; i < max_attempts; i++) {
        random_uuid_string(uuid, sizeof(uuid));
        if (!uuid_exists_in_file(uuids_path, uuid)) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Failed to generate a unique UUID after %d attempts\n", max_attempts);
        return NULL;
    }

    // Ajouter l'UUID au fichier (créer le fichier s'il n'existe pas)
    FILE* f = fopen(uuids_path, "a");
    if (!f) {
        perror("Failed to open uuids file for writing");
        return NULL;
    }
    fprintf(f, "%s\n", uuid);
    fclose(f);

    return strdup(uuid);
}