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

/**
 * Calcule la distance de Levenshtein entre deux chaînes (insensible à la casse)
 * @param s1 Première chaîne
 * @param s2 Deuxième chaîne
 * @return Distance de Levenshtein (0 = identique, plus c'est grand plus c'est différent)
 */
int levenshtein(const char* s1, const char* s2) {
    if (!s1 || !s2) return -1;
    
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    
    // Cas triviaux
    if (len1 == 0) return (int)len2;
    if (len2 == 0) return (int)len1;
    
    // Allouer la matrice de distances
    int** matrix = malloc((len1 + 1) * sizeof(int*));
    if (!matrix) return -1;
    
    for (size_t i = 0; i <= len1; i++) {
        matrix[i] = malloc((len2 + 1) * sizeof(int));
        if (!matrix[i]) {
            for (size_t j = 0; j < i; j++) free(matrix[j]);
            free(matrix);
            return -1;
        }
    }
    
    // Initialiser la première colonne et ligne
    for (size_t i = 0; i <= len1; i++) matrix[i][0] = (int)i;
    for (size_t j = 0; j <= len2; j++) matrix[0][j] = (int)j;
    
    // Remplir la matrice
    for (size_t i = 1; i <= len1; i++) {
        for (size_t j = 1; j <= len2; j++) {
            // Comparaison insensible à la casse
            int cost = (tolower((unsigned char)s1[i-1]) == tolower((unsigned char)s2[j-1])) ? 0 : 1;
            
            int del = matrix[i-1][j] + 1;       // Suppression
            int ins = matrix[i][j-1] + 1;       // Insertion
            int sub = matrix[i-1][j-1] + cost;  // Substitution
            
            // Minimum des trois opérations
            int min = del;
            if (ins < min) min = ins;
            if (sub < min) min = sub;
            
            matrix[i][j] = min;
        }
    }
    
    int result = matrix[len1][len2];
    
    // Libérer la mémoire
    for (size_t i = 0; i <= len1; i++) free(matrix[i]);
    free(matrix);
    
    return result;
}

/**
 * Vérifie si un mot est trop proche d'un mot de carte (à bannir)
 * @param input Le mot entré par le joueur
 * @param card_word Le mot sur la carte
 * @param threshold Seuil de distance max (ex: 2 = 2 modifications ou moins = trop proche)
 * @return 1 si le mot est trop proche (à bannir), 0 sinon
 */
int is_word_too_close(const char* input, const char* card_word, int threshold) {
    if (!input || !card_word) return 0;
    
    int distance = levenshtein(input, card_word);
    if (distance < 0) return 0;
    
    return distance <= threshold;
}

/**
 * Vérifie si un mot contient un autre mot (sous-chaîne, insensible à la casse)
 * @param input Le mot entré
 * @param card_word Le mot de la carte
 * @return 1 si input contient card_word ou vice versa, 0 sinon
 */
int word_contains(const char* input, const char* card_word) {
    if (!input || !card_word) return 0;
    
    size_t len1 = strlen(input);
    size_t len2 = strlen(card_word);
    
    // Créer des copies en minuscules
    char* lower1 = malloc(len1 + 1);
    char* lower2 = malloc(len2 + 1);
    if (!lower1 || !lower2) {
        free(lower1);
        free(lower2);
        return 0;
    }
    
    for (size_t i = 0; i <= len1; i++) lower1[i] = tolower((unsigned char)input[i]);
    for (size_t i = 0; i <= len2; i++) lower2[i] = tolower((unsigned char)card_word[i]);
    
    int result = (strstr(lower1, lower2) != NULL) || (strstr(lower2, lower1) != NULL);
    
    free(lower1);
    free(lower2);
    
    return result;
}