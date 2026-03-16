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
    if (!manager) return NULL;
    manager->lobbies = list_create();
    if (!manager->lobbies) {
        free(manager);
        return NULL;
    }
    return manager;
}

/** Callback interne pour détruire chaque lobby lors de la destruction du manager. */
static void destroy_lobby_callback(void* data, void* context) {
    Codenames* codenames = (Codenames*)context;
    Lobby* lobby = (Lobby*)data;
    if (lobby) {
        for (int i = 0; i < lobby->nb_players; i++) {
            char msg[2] = {(char)('0' + MSG_LOBBYCLOSED), '\0'};
            tcp_send_to_client(codenames, lobby->users[i]->socket_fd, msg);
            destroy_user(lobby->users[i]);
        }
        free(lobby);
    }
}

void destroy_lobby_manager(Codenames* codenames, LobbyManager* manager) {
    if (manager) {
        list_foreach(manager->lobbies, destroy_lobby_callback, codenames);
        list_destroy(manager->lobbies, NULL); // données déjà libérées par le callback
        free(manager);
    }
}

/** Extracteur d'ID pour list_next_available_id. */
static int lobby_get_id(void* data) {
    return ((Lobby*)data)->id;
}

Lobby* create_lobby(LobbyManager* manager) {
    if (list_size(manager->lobbies) >= MAX_LOBBIES) {
        return NULL;
    }
    Lobby* lobby = malloc(sizeof(Lobby));
    if (!lobby) return NULL;

    lobby->id = list_next_available_id(manager->lobbies, lobby_get_id);
    lobby->status = LB_STATUS_WAITING;
    lobby->nb_players = 0;
    lobby->owner_id = -1;
    lobby->game = NULL;
    strcpy(lobby->code, generate_code());

    if (list_add(manager->lobbies, lobby) != EXIT_SUCCESS) {
        free(lobby);
        return NULL;
    }

    return lobby;
}

int join_lobby(Lobby* lobby, User* user) {
    if (lobby->nb_players >= MAX_USERS) {
        return EXIT_FAILURE;
    }
    lobby->users[lobby->nb_players++] = user;
    return EXIT_SUCCESS;
}

/* Prédicats de recherche pour list_find */

static int predicate_owner_id(void* data, void* context) {
    Lobby* lobby = (Lobby*)data;
    int owner_id = *(int*)context;
    return lobby->owner_id == owner_id;
}

static int predicate_player_id(void* data, void* context) {
    Lobby* lobby = (Lobby*)data;
    int id = *(int*)context;
    for (int j = 0; j < lobby->nb_players; j++) {
        if (lobby->users[j]->id == id) return 1;
    }
    return 0;
}

static int predicate_code(void* data, void* context) {
    Lobby* lobby = (Lobby*)data;
    const char* code = (const char*)context;
    return strcmp(lobby->code, code) == 0;
}

Lobby* find_lobby_by_ownerid(LobbyManager* manager, int owner_id) {
    return (Lobby*)list_find(manager->lobbies, predicate_owner_id, &owner_id);
}

Lobby* find_lobby_by_playerid(LobbyManager* manager, int id) {
    return (Lobby*)list_find(manager->lobbies, predicate_player_id, &id);
}

Lobby* find_lobby_by_code(LobbyManager* manager, const char* code) {
    return (Lobby*)list_find(manager->lobbies, predicate_code, (void*)code);
}

void destroy_lobby(Codenames* codenames, Lobby* lobby) {
    if (lobby) {
        for (int i = 0; i < lobby->nb_players; i++) {
            char msg[2] = {(char)('0' + MSG_LOBBYCLOSED), '\0'};
            tcp_send_to_client(codenames, lobby->users[i]->socket_fd, msg);
            destroy_user(lobby->users[i]);
        }
        list_remove(codenames->lobby->lobbies, lobby);
        free(lobby);
    }
}

// interactions joueurs
int request_create_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    if (args.argc < 1) {
        printf("Invalid create lobby message from client %d: \"%s\"\n", client->id, message);
        return EXIT_FAILURE;
    }
    Lobby* lobby = create_lobby(codenames->lobby);
    if (lobby == NULL) {
        printf("Failed to create lobby\n");
        return EXIT_FAILURE;
    }
    lobby->owner_id = client->id;
    User* user = create_user(client->id, args.argv[0], client->socket);
    if (user == NULL) {
        printf("Failed to create user for client %d\n", client->id);
        return EXIT_FAILURE;
    }
    if (join_lobby(lobby, user) != EXIT_SUCCESS) {
        printf("Failed to join lobby for client %d\n", client->id);
        destroy_user(user);
        return EXIT_FAILURE;
    }
    printf("create lobby %d\n", lobby->id);

    char reponse[64];
    format_to(reponse, sizeof(reponse), "%d %d %s", MSG_CREATELOBBY, lobby->id, lobby->code);
    tcp_send_to_client(codenames, client->id, reponse);

    return EXIT_SUCCESS;
}

int request_join_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    // Handle join lobby
    if (args.argc < 2) {
        printf("Invalid join lobby message from client %d: \"%s\"\n", client->id, message);
        return EXIT_FAILURE;
    }

    Lobby* lobby = find_lobby_by_code(codenames->lobby, args.argv[0]);
    if (!lobby) {
        printf("Lobby not found for client %d\n", client->id);
        return EXIT_FAILURE;
    }

    User* user = create_user(client->id, args.argv[1], client->socket);
    if (!user) {
        printf("Failed to create user for client %d\n", client->id);
        return EXIT_FAILURE;
    }

    if (join_lobby(lobby, user) != EXIT_SUCCESS) {
        printf("Failed to join lobby %d for client %d\n", lobby->id, client->id);
        destroy_user(user);
        return EXIT_FAILURE;
    }

    printf("Client %d joined lobby %d\n", client->id, lobby->id);

    return EXIT_SUCCESS;
}

int request_leave_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    (void)message; // unused
    (void)args;    // unused

    /* Trouver le lobby où se trouve le client */
    Lobby* lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d tried to leave but is not in any lobby\n", client->id);
        return EXIT_FAILURE;
    }

    int lobby_id = lobby->id;

    /* Trouver et retirer l'utilisateur du lobby */
    for (int i = 0; i < lobby->nb_players; i++) {
        if (lobby->users[i]->id == client->id) {
            destroy_user(lobby->users[i]);
            /* Décaler les utilisateurs suivants */
            for (int j = i; j < lobby->nb_players - 1; j++) {
                lobby->users[j] = lobby->users[j + 1];
            }
            lobby->users[lobby->nb_players - 1] = NULL;
            lobby->nb_players--;
            break;
        }
    }

    printf("Client %d left lobby %d\n", client->id, lobby_id);

    /* Si le lobby est vide, le détruire */
    if (lobby->nb_players == 0) {
        list_remove(codenames->lobby->lobbies, lobby);
        free(lobby);
        printf("Lobby %d destroyed (empty)\n", lobby_id);
    }
    /* Si le owner a quitté, transférer au premier joueur restant */
    else if (lobby->owner_id == client->id) {
        lobby->owner_id = lobby->users[0]->id;
        printf("Lobby %d ownership transferred to client %d\n", lobby_id, lobby->owner_id);
    }

    return EXIT_SUCCESS;
}