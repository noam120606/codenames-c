#include "../lib/all.h"

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

char* itoa(int value) {
    static char buffer[20]; 
    snprintf(buffer, sizeof(buffer), "%d", value);
    return buffer;
}

char* getRandomUsername(void) {
    FILE* f = fopen("assets/misc/usernames.txt", "r");
    if (!f) return NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char** names = NULL;
    size_t count = 0;
    while ((read = getline(&line, &len, f)) != -1) {
        if (read > 0 && line[read - 1] == '\n') line[read - 1] = '\0';
        char* s = strdup(line);
        if (!s) {
            free(line);
            for (size_t i = 0; i < count; i++) free(names[i]);
            free(names);
            fclose(f);
            return NULL;
        }
        char** tmp = realloc(names, sizeof(char*) * (count + 1));
        if (!tmp) {
            free(s);
            free(line);
            for (size_t i = 0; i < count; i++) free(names[i]);
            free(names);
            fclose(f);
            return NULL;
        }
        names = tmp;
        names[count++] = s;
    }
    free(line);
    fclose(f);
    if (count == 0) {
        free(names);
        return NULL;
    }
    int idx = rand() % count;
    char* result = strdup(names[idx]);
    for (size_t i = 0; i < count; i++) free(names[i]);
    free(names);
    return result;
}