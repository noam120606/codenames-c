#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/game.h"
#include "../lib/utils.h"
#include "../lib/tcp.h"
#include "../lib/codenames.h"

int WORDCOUNT = 0;

/* initialise WORDCOUNT en comptant les mots dans assets/wordlist.txt.
   retourne EXIT_SUCCESS si OK, EXIT_FAILURE en cas d'erreur */
int init_game_manager(void) {
    int n = count_words("assets/wordlist.txt");
    if (n < 0) {
        fprintf(stderr, "game_manager: failed to count words in assets/wordlist.txt\n");
        return EXIT_FAILURE;
    }
    WORDCOUNT = n;
    return EXIT_SUCCESS;
}

char** fetchWords() { // Lit le fichier wordlist.txt et ordonne dans une liste dont l'adresse est renvoyée
    
    FILE* file = fopen("assets/wordlist.txt", "r");
    if (!file) {
        perror("Failed to open words file");
        return NULL;
    }
    char** words = (char**)malloc(sizeof(char*) * WORDCOUNT);
    if (!words) {
        perror("Failed to allocate memory for words");
        fclose(file);
        return NULL;
    }
    char buffer[32];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), file) && count < WORDCOUNT) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
        words[count] = (char*)malloc(strlen(buffer) + 1);
        if (!words[count]) {
            perror("Failed to allocate memory for a word");
            for (int j = 0; j < count; j++) {
                free(words[j]);
            }
            free(words);
            fclose(file);
            return NULL;
        }
        strcpy(words[count], buffer);
        count++;
    }
    fclose(file);
    return words;
}

Word* generateWords(int count) { //Trouver 25 mots au hasard parmis la liste des mots.
    Word* words = (Word*)malloc(sizeof(Word) * count);
    if (!words) {
        perror("Failed to allocate memory for words");
        return NULL;
    }

    char ** sample_words = fetchWords();
    if (!sample_words) {
        free(words);
        return NULL;
    }

    int red_count = count / 3 + 1; // Example distribution
    int blue_count = count / 3;
    int neutral_count = count - red_count - blue_count - 1;

    int already_used[WORDCOUNT];
    memset(already_used, 0, sizeof(already_used));

    for (int i = 0; i < count; i++) {
        int index;
        do {
            index = randint(0, WORDCOUNT - 1);
        } while (already_used[index]);
        already_used[index] = 1;

        strncpy(words[i].word, sample_words[index], sizeof(words[i].word) - 1);
        words[i].word[sizeof(words[i].word) - 1] = '\0';
        words[i].team = (i < red_count) ? TEAM_RED :
                        (i < red_count + blue_count) ? TEAM_BLUE :
                        (i < red_count + blue_count + neutral_count) ? TEAM_NEUTRAL : TEAM_BLACK;
        words[i].revealed = 0;
    }

    return words;
}

void shuffleWords(Word* words, int count) { // Mélange les cartes auxquelles sont attribuées des couleurs rangées dans l'ordre
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Word temp = words[i];
        words[i] = words[j];
        words[j] = temp;
    }
}
