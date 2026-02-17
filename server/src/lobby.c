#include "../lib/all.h"


int USERNAMESCOUNT = 0;

/* initialise USERNAMESCOUNT en comptant les mots dans assets/usernames.txt.
   retourne EXIT_SUCCESS si OK, EXIT_FAILURE en cas d'erreur */
int init_lobby_manager() {
    int n = count_words("assets/usernames.txt");
    if (n < 0) {
        fprintf(stderr, "lobby_manager: failed to count words in assets/usernames.txt\n");
        return EXIT_FAILURE;
    }
    USERNAMESCOUNT = n;
    return EXIT_SUCCESS;
}

char** fetchUsernames() { // Lit le fichier wordlist.txt et ordonne dans une liste dont l'adresse est renvoyée
    
    FILE* file = fopen("assets/usernames.txt", "r");
    if (!file) {
        perror("Failed to open usernames file");
        return NULL;
    }
    char** usernames = (char**)malloc(sizeof(char*) * USERNAMESCOUNT);
    if (!usernames) {
        perror("Failed to allocate memory for usernames");
        fclose(file);
        return NULL;
    }
    char buffer[32];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), file) && count < USERNAMESCOUNT) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
        usernames[count] = (char*)malloc(strlen(buffer) + 1);
        if (!usernames[count]) {
            perror("Failed to allocate memory for a username");
            for (int j = 0; j < count; j++) {
                free(usernames[j]);
            }
            free(usernames);
            fclose(file);
            return NULL;
        }
        strcpy(usernames[count], buffer);
        count++;
    }
    fclose(file);
    return usernames;
}

char* getRandomUsername() {
    char** usernames = fetchUsernames();
    if (!usernames) {
        return NULL;
    }
    int index = rand() % USERNAMESCOUNT;
    char* username = strdup(usernames[index]);
    for (int i = 0; i < USERNAMESCOUNT; i++) {
        free(usernames[i]);
    }
    free(usernames);
    return username;
}

LobbyManager* create_lobby_manager() {
    LobbyManager* manager = malloc(sizeof(LobbyManager));
    manager->nb_lobbies = 0;
    return manager;
}

void destroy_lobby_manager(Codenames* codenames, LobbyManager* manager) {
    if (manager) {
        for (int i = 0; i < manager->nb_lobbies; i++) {
            destroy_lobby(codenames, manager->lobbies[i]);
        }
        free(manager);
    }
}

Lobby* create_lobby(LobbyManager* manager) {
    if (manager->nb_lobbies >= MAX_LOBBIES) {
        return NULL;
    }
    Lobby* lobby = malloc(sizeof(Lobby));
    lobby->id = manager->nb_lobbies;
    lobby->status = LB_STATUS_WAITING;
    lobby->nb_players = 0;
    lobby->owner_id = -1;
    lobby->game = NULL;

    manager->lobbies[manager->nb_lobbies++] = lobby;
    
    return lobby;
}

int join_lobby(Lobby* lobby, User* user) {
    if (lobby->nb_players >= MAX_USERS) {
        return EXIT_FAILURE;
    }
    lobby->users[lobby->nb_players++] = user;
    return EXIT_SUCCESS;
}

Lobby* find_lobby_by_ownerid(LobbyManager* manager, int owner_id) {
    for (int i = 0; i < manager->nb_lobbies; i++) {
        if (manager->lobbies[i]->owner_id == owner_id) {
            return manager->lobbies[i];
        }
    }
    return NULL;
}

void destroy_lobby(Codenames* codenames, Lobby* lobby) {
    if (lobby) {
        for (int i = 0; i < lobby->nb_players; i++) {
            char msg[2] = {(char)('0' + MSG_LOBBYCLOSED), '\0'};
            tcp_send_to_client(codenames, lobby->users[i]->socket_fd, msg);
            destroy_user(lobby->users[i]);
        }
        free(lobby);
    }
}