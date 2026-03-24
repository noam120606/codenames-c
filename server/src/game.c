#include "../lib/all.h"

int WORDCOUNT = 0;

/* initialise WORDCOUNT en comptant les mots dans assets/wordlist.txt.
   retourne EXIT_SUCCESS si OK, EXIT_FAILURE en cas d'erreur */
int init_game_manager() {
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

Word* generateWords(int count, Team start_team) { //Trouver 25 mots au hasard parmis la liste des mots.
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

    int red_count = count / 3 + (start_team == TEAM_RED ? 1 : 0);
    int blue_count = count / 3 + (start_team == TEAM_BLUE ? 1 : 0);
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
                        (i < red_count + blue_count + neutral_count) ? TEAM_NONE : TEAM_BLACK;
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

int destroy_game(Game* game) {
    if (!game) return EXIT_FAILURE;
    if (game->words) {
        for (int i = 0; i < game->nb_words; i++) {
            free(game->words[i].word);
        }
        free(game->words);
    }
    free(game);
    return EXIT_SUCCESS;
}

int request_start_game(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    // Vérifie que le client est bien dans un lobby
    Lobby* lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d doesn't own a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be the lobby owner to start the game");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby est prêt à démarrer
    if (lobby->status != LB_STATUS_WAITING) {
        printf("Lobby %d is not ready to start\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Lobby is not ready to start");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Démarre le jeu
    lobby->status = LB_STATUS_IN_GAME;
    printf("Game started in lobby %d\n", lobby->id);

    Game* game = (Game*)malloc(sizeof(Game));
    if (!game) {
        printf("Failed to allocate memory for game\n");
        return EXIT_FAILURE;
    }

    Team start_team = (rand() % 2 == 0) ? TEAM_RED : TEAM_BLUE;

    // Génère les mots pour le jeu
    game->words = generateWords(25, start_team);
    shuffleWords(game->words, 25);
    if (!game->words) {
        printf("Failed to generate words for lobby %d\n", lobby->id);
        destroy_game(game);
        return EXIT_FAILURE;
    }

    game->nb_words = 25;
    game->state = start_team == TEAM_RED ? GAMESTATE_TURN_RED_SPY : GAMESTATE_TURN_BLUE_SPY;
    lobby->game = game;

    // Envoi de la partie aux joueurs
    for (int i = 0; i < lobby->nb_players; i++) {
        User* user = lobby->users[i];
        char msg[32];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_STARTGAME, game->state, game->nb_words);
        tcp_send_to_client(codenames, user->id, msg);
        // Envoi de chaque mot
        for (int j = 0; j < game->nb_words; j++) {
            format_to(msg, sizeof(msg), "%d %d %s %d %d", MSG_WORDDATA, j, game->words[j].word, game->words[j].team, game->words[j].revealed);
            tcp_send_to_client(codenames, user->id, msg);
        }
    }

    return EXIT_SUCCESS;
}