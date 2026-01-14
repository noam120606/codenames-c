#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/game_manager.h"
#include "../include/utils.h"

const int WORDCOUNT = 400;

char** fetchWords() {
    
    FILE* file = fopen("storage/wordlist.txt", "r");
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


Word* generateWords(int count) {
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
                        (i < red_count + blue_count + neutral_count) ? TEAM_NEUTRAL : TEAM_ASSASSIN;
        words[i].revealed = 0;
    }

    return words;
}

void shuffleWords(Word* words, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Word temp = words[i];
        words[i] = words[j];
        words[j] = temp;
    }
}

Game* create_game(int game_id, int word_count) {
    Word* words = generateWords(word_count);
    if (!words) {
        return NULL;
    }
    shuffleWords(words, word_count);
    Game* new_game = (Game*)malloc(sizeof(Game));
    if (!new_game) {
        perror("Failed to allocate memory for new game");
        return NULL;
    }
    new_game->game_id = game_id;
    new_game->words = words;
    new_game->word_count = word_count;
    new_game->status = GAME_STATUS_WAITING;
    new_game->red_words_remaining = 0;
    new_game->blue_words_remaining = 0;

    // Count the number of red and blue words
    for (int i = 0; i < word_count; i++) {
        if (words[i].team == TEAM_RED) {
            new_game->red_words_remaining++;
        } else if (words[i].team == TEAM_BLUE) {
            new_game->blue_words_remaining++;
        }
    }

    return new_game;
}

int destroy_game(Game* game) {
    if (!game) {
        return EXIT_FAILURE;
    }
    
    if (game->words) {
        free(game->words); // fuite de memoire a verif
    }

    free(game);
    return EXIT_SUCCESS;
}

// Function to reveal a word in the game
int reveal_word(Game* game, int word_id) {
    if (!game || !game->words) {
        return EXIT_FAILURE;
    }

    
    if (game->words[word_id].revealed) {
        return EXIT_FAILURE; // Word already revealed
    }
    game->words[word_id].revealed = 1;

    // Update remaining words count
    if (game->words[word_id].team == TEAM_RED) {
        game->red_words_remaining--;
    } else if (game->words[word_id].team == TEAM_BLUE) {
        game->blue_words_remaining--;
    }

    return EXIT_SUCCESS;
}

// Function to check if the game has ended
int check_game_end(Game* game) {
    if (!game) {
        return 0;
    }

    if (game->red_words_remaining == 0) {
        game->status = GAME_STATUS_ENDED;
        return TEAM_RED;
    } else if (game->blue_words_remaining == 0) {
        game->status = GAME_STATUS_ENDED;
        return TEAM_BLUE;
    }

    return 0;
}

// Convert game to json string
char* game_to_json(Game* game) {
    if (!game) {
        return NULL;
    }

    // Estimate the size needed for the JSON string
    int json_size = 256 + game->word_count * 64;
    char* json_str = (char*)malloc(json_size);
    if (!json_str) {
        perror("Failed to allocate memory for JSON string");
        return NULL;
    }

    snprintf(json_str, json_size,
             "{\"game_id\":%d,\"status\":%d,\"red_words_remaining\":%d,\"blue_words_remaining\":%d,\"words\":[",
             game->game_id, game->status, game->red_words_remaining, game->blue_words_remaining);

    for (int i = 0; i < game->word_count; i++) {
        char word_json[128];
        snprintf(word_json, sizeof(word_json),
                 "{\"word\":\"%s\",\"team\":%d,\"revealed\":%d}%s",
                 game->words[i].word, game->words[i].team,
                 game->words[i].revealed, (i < game->word_count - 1) ? "," : "");
        strncat(json_str, word_json, json_size - strlen(json_str) - 1);
    }

    strncat(json_str, "]}", json_size - strlen(json_str) - 1);

    return json_str;
}

// Function to print game details (for debugging)
void print_game(Game* game) {
    if (!game) {
        printf("Game is NULL\n");
        return;
    }
    printf("Game ID: %d\n", game->game_id);
    printf("Status: %d\n", game->status);
    printf("Red Words Remaining: %d\n", game->red_words_remaining);
    printf("Blue Words Remaining: %d\n", game->blue_words_remaining);
    printf("Words:\n");
    for (int i = 0; i < game->word_count; i++) {
        printf("  Word: %s, Team: %d, Revealed: %d\n",
               game->words[i].word, game->words[i].team, game->words[i].revealed);
    }
}

GameManager* create_game_manager() {
    GameManager* manager = (GameManager*)malloc(sizeof(GameManager));
    if (!manager) {
        perror("Failed to allocate memory for GameManager");
        return NULL;
    }
    manager->lobbies = NULL;
    manager->lobby_count = 0;
    return manager;
}

int destroy_game_manager(GameManager* manager) {
    if (!manager) {
        return EXIT_FAILURE;
    }

    // Free each lobby
    for (int i = 0; i < manager->lobby_count; i++) {
        Lobby* lobby = &manager->lobbies[i];
        // Free players in the lobby
        if (lobby->players) {
            free(lobby->players);
        }
        // Free the game in the lobby
        destroy_game(&lobby->game);
    }

    // Free the lobbies array
    if (manager->lobbies) {
        free(manager->lobbies);
    }

    free(manager);
    return EXIT_SUCCESS;
}

int add_game_manager_lobby(GameManager* manager, Lobby new_lobby) {
    if (!manager) {
        return EXIT_FAILURE;
    }

    Lobby* updated_lobbies = (Lobby*)realloc(manager->lobbies, sizeof(Lobby) * (manager->lobby_count + 1));
    if (!updated_lobbies) {
        perror("Failed to reallocate memory for lobbies");
        return EXIT_FAILURE;
    }

    manager->lobbies = updated_lobbies;
    manager->lobbies[manager->lobby_count] = new_lobby;
    manager->lobby_count++;

    return EXIT_SUCCESS;
}


// Fonctions publiques

Lobby* new_lobby(GameManager* manager, int max_players) {
    if (!manager) {
        return NULL;
    }

    Lobby new_lobby;
    new_lobby.lobby_id = manager->lobby_count + 1;
    new_lobby.players = NULL;
    new_lobby.current_players = 0;
    new_lobby.max_players = max_players;
    new_lobby.game.game_id = 0; // No game yet

    if (add_game_manager_lobby(manager, new_lobby) != EXIT_SUCCESS) {
        return NULL;
    }

    return &manager->lobbies[manager->lobby_count - 1];
}
void new_game(GameManager* manager, int lobby_id) {
    if (!manager) {
        return;
    }

    for (int i = 0; i < manager->lobby_count; i++) {
        if (manager->lobbies[i].lobby_id == lobby_id) {
            Lobby* lobby = &manager->lobbies[i];
            if (lobby->game.game_id != 0) {
                // Game already exists
                return;
            }
            Game* game = create_game(lobby_id, 25);
            if (!game) {
                return;
            }
            lobby->game = *game;
            free(game);
            return;
        }
    }
}