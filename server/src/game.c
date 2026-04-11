#include "../lib/all.h"

int WORDCOUNT_NORMAL = 0;
int WORDCOUNT_HARD = 0;
int WORDCOUNT_INFO = 0;
int WORDCOUNT_FREAKY = 0;

/* Initialise les compteurs de mots pour les deux niveaux de difficulté.
   Retourne EXIT_SUCCESS si OK, EXIT_FAILURE en cas d'erreur */
int init_game_manager() {
    int n = count_words("assets/wordlist.txt");
    if (n < 0) {
        fprintf(stderr, "game_manager: failed to count words in assets/wordlist.txt\n");
        return EXIT_FAILURE;
    }
    WORDCOUNT_NORMAL = n;

    n = count_words("assets/wordlist_hard.txt");
    if (n < 0) {
        fprintf(stderr, "game_manager: failed to count words in assets/wordlist_hard.txt\n");
        return EXIT_FAILURE;
    }
    WORDCOUNT_HARD = n;

    n = count_words("assets/wordlist_info.txt");
    if (n < 0) {
        fprintf(stderr, "game_manager: failed to count words in assets/wordlist_info.txt\n");
        return EXIT_FAILURE;
    }
    WORDCOUNT_INFO = n;

    n = count_words("assets/wordlist_freaky.txt");
    if (n < 0) {
        fprintf(stderr, "game_manager: failed to count words in assets/wordlist_freaky.txt\n");
        return EXIT_FAILURE;
    }
    WORDCOUNT_FREAKY = n;

    return EXIT_SUCCESS;
}

char** fetchWords(WordsDifficulty words_difficulty) { // Lit le fichier wordlist selon la difficulté
    const char* filepath = (words_difficulty == WORDS_DIFFICULTY_NORMAL) ? "assets/wordlist.txt" :
                           (words_difficulty == WORDS_DIFFICULTY_HARD) ? "assets/wordlist_hard.txt" :
                           (words_difficulty == WORDS_DIFFICULTY_INFO) ? "assets/wordlist_info.txt" :
                           "assets/wordlist_freaky.txt";
    int wordcount = (words_difficulty == WORDS_DIFFICULTY_NORMAL) ? WORDCOUNT_NORMAL :
                    (words_difficulty == WORDS_DIFFICULTY_HARD) ? WORDCOUNT_HARD :
                    (words_difficulty == WORDS_DIFFICULTY_INFO) ? WORDCOUNT_INFO :
                    WORDCOUNT_FREAKY;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Failed to open words file");
        return NULL;
    }
    char** words = (char**)malloc(sizeof(char*) * wordcount);
    if (!words) {
        perror("Failed to allocate memory for words");
        fclose(file);
        return NULL;
    }
    char buffer[32];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), file) && count < wordcount) {
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

Word* generateWords(int count, Team start_team, WordsDifficulty words_difficulty, int nb_assassins) { //Trouver 25 mots au hasard parmis la liste des mots.
    int wordcount = (words_difficulty == WORDS_DIFFICULTY_NORMAL) ? WORDCOUNT_NORMAL :
                    (words_difficulty == WORDS_DIFFICULTY_HARD) ? WORDCOUNT_HARD :
                    (words_difficulty == WORDS_DIFFICULTY_INFO) ? WORDCOUNT_INFO :
                    WORDCOUNT_FREAKY;

    Word* words = (Word*)malloc(sizeof(Word) * count);
    if (!words) {
        perror("Failed to allocate memory for words");
        return NULL;
    }

    char ** sample_words = fetchWords(words_difficulty);
    if (!sample_words) {
        free(words);
        return NULL;
    }

    if (nb_assassins < 1) nb_assassins = 1;
    if (nb_assassins > 3) nb_assassins = 3;

    int red_count = count / 3 + (start_team == TEAM_RED ? 1 : 0);
    int blue_count = count / 3 + (start_team == TEAM_BLUE ? 1 : 0);
    int neutral_count = count - red_count - blue_count - nb_assassins;
    if (neutral_count < 0) neutral_count = 0;

    int* already_used = calloc(wordcount, sizeof(int));
    if (!already_used) {
        perror("Failed to allocate memory for already_used");
        free(words);
        for (int j = 0; j < wordcount; j++) free(sample_words[j]);
        free(sample_words);
        return NULL;
    }

    // Sélectionne aléatoirement des mots dans la liste et les attribue à une équipe
    for (int i = 0; i < count; i++) {
        int index;
        do {
            index = randint(0, wordcount - 1);
        } while (already_used[index]);
        already_used[index] = 1;

        strncpy(words[i].word, sample_words[index], sizeof(words[i].word) - 1);
        words[i].word[sizeof(words[i].word) - 1] = '\0';
        words[i].team = (i < red_count) ? TEAM_RED :
            (i < red_count + blue_count) ? TEAM_BLUE :
            (i < red_count + blue_count + neutral_count) ? TEAM_NONE : TEAM_BLACK;
        words[i].type = (CardType)(rand() % 4); // Attribue un type de carte aléatoire
        words[i].revealed = 0;
    }

    // Libère la mémoire des mots chargés et du tableau already_used
    for (int j = 0; j < wordcount; j++) free(sample_words[j]);
    free(sample_words);
    free(already_used);

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
        free(game->words);
        game->words = NULL;
    }

    free(game);
    return EXIT_SUCCESS;
}

int request_start_game(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;
    (void)args;

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
    if (lobby->game) {
        destroy_game(lobby->game);
        lobby->game = NULL;
    }

    lobby->status = LB_STATUS_IN_GAME;
    printf("Game started in lobby %d\n", lobby->id);

    Game* game = (Game*)malloc(sizeof(Game));
    if (!game) {
        printf("Failed to allocate memory for game\n");
        return EXIT_FAILURE;
    }

    Team start_team = (rand() % 2 == 0) ? TEAM_RED : TEAM_BLUE;

    // Génère les mots pour le jeu avec la difficulté des mots choisie
    game->words = generateWords(25, start_team, lobby->words_difficulty, lobby->nb_assassins);
    shuffleWords(game->words, 25);
    if (!game->words) {
        printf("Failed to generate words for lobby %d\n", lobby->id);
        destroy_game(game);
        return EXIT_FAILURE;
    }

    game->nb_words = 25;
    game->state = start_team == TEAM_RED ? GAMESTATE_TURN_RED_SPY : GAMESTATE_TURN_BLUE_SPY;
    game->can_guess = 0;
    lobby->game = game;

    // Envoi de la partie aux joueurs
    for (int i = 0; i < lobby->nb_players; i++) {
        User* user = lobby->users[i];
        char msg[32];
        format_to(msg, sizeof(msg), "%d %d %d", MSG_STARTGAME, game->state, game->nb_words);
        tcp_send_to_client(codenames, user->id, msg);
        // Envoi de chaque mot
        for (int j = 0; j < game->nb_words; j++) {
            format_to(msg, sizeof(msg), "%d %d %s %d %d %d", MSG_WORDDATA, j, game->words[j].word, game->words[j].team, game->words[j].type, game->words[j].revealed);
            tcp_send_to_client(codenames, user->id, msg);
        }
    }

    return EXIT_SUCCESS;
}

int request_submit_hint(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;
    
    // Vérifie que le client est bien dans un lobby
    Lobby* lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d is not in a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be in a lobby to submit a hint");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby est bien en partie
    if (lobby->status != LB_STATUS_IN_GAME || !lobby->game) {
        printf("Lobby %d is not in game\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "No game in progress");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que c'est bien le tour d'un espion
    if (lobby->game->state != GAMESTATE_TURN_RED_SPY && lobby->game->state != GAMESTATE_TURN_BLUE_SPY) {
        printf("It's not the spy's turn in lobby %d\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "It's not the spy's turn");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie les arguments: nb_hint et hint_word
    if (args.argc < 2) {
        printf("Invalid submit hint from client %d : \"%s\"\n", client->id, message);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid hint format");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    int nb_hint = atoi((char*)args.argv[0]);
    char* hint_word = (char*)args.argv[1];

    printf("Client %d submitted hint: %s (%d)\n", client->id, hint_word, nb_hint);

    // Change le gamestate de SPY à AGENT
    GameState new_state;
    if (lobby->game->state == GAMESTATE_TURN_RED_SPY) {
        new_state = GAMESTATE_TURN_RED_AGENT;
    } else {
        new_state = GAMESTATE_TURN_BLUE_AGENT;
    }
    lobby->game->state = new_state;
    lobby->game->can_guess = nb_hint + 1;

    printf("Game state changed to %d in lobby %d\n", new_state, lobby->id);

    // Diffuse l'indice et le nouveau gamestate à tous les joueurs du lobby
    char msg[128];
    format_to(msg, sizeof(msg), "%d %d %d %s %d", MSG_SUBMIT_HINT, client->id, nb_hint, hint_word, new_state);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;
}

int count_remaining_words(Game* game, Team team) {
    int count = 0;
    for (int i = 0; i < game->nb_words; i++) {
        if (game->words[i].team == team && !game->words[i].revealed) {
            count++;
        }
    }
    return count;
}

int request_preguess(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;

    // Vérifie que le client est bien dans un lobby
    Lobby* lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d is not in a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be in a lobby to pre-guess a card");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby est bien en partie
    if (lobby->status != LB_STATUS_IN_GAME || !lobby->game) {
        printf("Lobby %d is not in game\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "No game in progress");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    int word_index = atoi((char*)args.argv[0]);
    int selected = atoi((char*)args.argv[1]);

    // Diffuse la carte sélectionné et le nouveau gamestate à tous les joueurs du lobby
    char msg[64];
    format_to(msg, sizeof(msg), "%d %d %d %d", MSG_PREGUESS, word_index, selected, client->id);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;

}

int request_guess_card(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    // Vérifie que le client est bien dans un lobby
    Lobby* lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d is not in a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be in a lobby to guess a card");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby est bien en partie
    if (lobby->status != LB_STATUS_IN_GAME || !lobby->game) {
        printf("Lobby %d is not in game\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "No game in progress");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    User* guessing_user = find_user_by_id(lobby, client->id);

    // Vérifie que c'est bien le tour d'un agent
    if (lobby->game->state != GAMESTATE_TURN_RED_AGENT && lobby->game->state != GAMESTATE_TURN_BLUE_AGENT) {
        printf("It's not the agent's turn in lobby %d\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "It's not the agent's turn");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie les arguments: card_index
    if (args.argc < 1) {
        printf("Invalid guess card from client %d: \"%s\"\n", client->id, message);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid card index");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    const char* guessing_name = (args.argc >= 2) ? (char*)args.argv[1] : NULL;
    if ((!guessing_name || guessing_name[0] == '\0') && guessing_user && guessing_user->name && guessing_user->name[0] != '\0') {
        guessing_name = guessing_user->name;
    }
    if (!guessing_name || guessing_name[0] == '\0') {
        guessing_name = "Agent";
    }

    int word_index = atoi((char*)args.argv[0]);
    if (word_index < -1 || word_index >= 25) {
        printf("Invalid word index from client %d: %d\n", client->id, word_index);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid word index");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    if (lobby->game->can_guess <= 0) {
        printf("No guesses left for client %d in lobby %d\n", client->id, lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "No guesses left");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    if (word_index == -1) {
        printf("Client %d ended their turn in lobby %d\n", client->id, lobby->id);
        // Change le gamestate de AGENT à SPY
        GameState new_state;
        if (lobby->game->state == GAMESTATE_TURN_RED_AGENT) {
            new_state = GAMESTATE_TURN_BLUE_SPY;
        } else {
            new_state = GAMESTATE_TURN_RED_SPY;
        }
        lobby->game->state = new_state;

        printf("Game state changed to %d in lobby %d\n", new_state, lobby->id);

        // Diffuse le nouveau gamestate à tous les joueurs du lobby
        char msg[128];
        format_to(msg, sizeof(msg), "%d %d %d %s", MSG_GUESS_CARD, -1, new_state, guessing_name);
        for (int i = 0; i < lobby->nb_players; i++) {
            tcp_send_to_client(codenames, lobby->users[i]->id, msg);
        }
        return EXIT_SUCCESS;
    }

    Word* word = lobby->game->words + word_index;

    // Vérifie que la carte n'est pas déjà révélée
    if (word->revealed) {
        printf("Word \"%s\" is already revealed\n", word->word);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Word is already revealed");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Révèle la carte
    lobby->game->can_guess--;
    word->revealed = 1;

    printf("Word \"%s\" guessed by client %d\n", word->word, client->id);

    Team winner = TEAM_NONE;

    // Change le gamestate de AGENT à SPY
    GameState new_state = lobby->game->state;
    Team current_team = (lobby->game->state == GAMESTATE_TURN_RED_AGENT) ? TEAM_RED : TEAM_BLUE;
    if (lobby->game->can_guess <= 0 || word->team != current_team) {
        if (lobby->game->state == GAMESTATE_TURN_RED_AGENT) {
            new_state = GAMESTATE_TURN_BLUE_SPY;
        } else if (lobby->game->state == GAMESTATE_TURN_BLUE_AGENT) {
            new_state = GAMESTATE_TURN_RED_SPY;
        }
    }
    if (word->team == TEAM_BLACK) {
        new_state = GAMESTATE_ENDED;
        winner = (current_team == TEAM_RED) ? TEAM_BLUE : TEAM_RED;
    }
    if (count_remaining_words(lobby->game, TEAM_RED) == 0) {
        new_state = GAMESTATE_ENDED;
        winner = TEAM_RED;
    }
    if (count_remaining_words(lobby->game, TEAM_BLUE) == 0) {
        new_state = GAMESTATE_ENDED;
        winner = TEAM_BLUE;
    }
    lobby->game->state = new_state;

    if (new_state == GAMESTATE_ENDED) {
        lobby->status = LB_STATUS_WAITING;
        lobby->game->can_guess = 0;
        printf("Lobby %d returned to waiting state after end of game\n", lobby->id);
    }

    printf("Game state changed to %d in lobby %d\n", new_state, lobby->id);

    // Diffuse la carte révélée et le nouveau gamestate à tous les joueurs du lobby
    char msg[128];
    format_to(msg, sizeof(msg), "%d %d %d %d %s", MSG_GUESS_CARD, word_index, new_state, winner, guessing_name);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;
}

int request_set_words_difficulty(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;
    
    // Vérifie que le client est bien propriétaire d'un lobby
    Lobby* lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d doesn't own a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be the lobby owner to change difficulty");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby n'est pas en partie
    if (lobby->status != LB_STATUS_WAITING) {
        printf("Cannot change difficulty while game is in progress in lobby %d\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Cannot change difficulty while game is in progress");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Récupère la difficulté demandée
    if (args.argc < 1) {
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Missing difficulty argument");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    int words_difficulty = atoi((char*)args.argv[0]);
    if (words_difficulty != WORDS_DIFFICULTY_NORMAL && 
        words_difficulty != WORDS_DIFFICULTY_HARD && 
        words_difficulty != WORDS_DIFFICULTY_INFO && 
        words_difficulty != WORDS_DIFFICULTY_FREAKY
    ) {
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid difficulty value");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    lobby->words_difficulty = (WordsDifficulty)words_difficulty;
    printf("Lobby %d words difficulty set to %d\n", lobby->id, words_difficulty);

    // Informe tous les joueurs du changement de difficulté
    char msg[32];
    format_to(msg, sizeof(msg), "%d %d", MSG_SET_WORDS_DIFFICULTY, words_difficulty);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;
}

int request_set_nb_assassins(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message;

    // Vérifie que le client est bien propriétaire d'un lobby
    Lobby* lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d doesn't own a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be the lobby owner to change nb_assassins");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le lobby n'est pas en partie
    if (lobby->status != LB_STATUS_WAITING) {
        printf("Cannot change nb_assassins while game is in progress in lobby %d\n", lobby->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Cannot change nb_assassins while game is in progress");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    if (args.argc < 1) {
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Missing nb_assassins argument");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    int nb_assassins = atoi((char*)args.argv[0]);
    if (nb_assassins < 1 || nb_assassins > 3) {
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid nb_assassins value");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    lobby->nb_assassins = nb_assassins;
    printf("Lobby %d nb_assassins set to %d\n", lobby->id, nb_assassins);

    // Informe tous les joueurs du changement
    char msg[32];
    format_to(msg, sizeof(msg), "%d %d", MSG_SET_NB_ASSASSINS, nb_assassins);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;
}