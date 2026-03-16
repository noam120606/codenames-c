#include "../lib/all.h"

MessageType fetch_header(char* message) {
    MessageType header;
    if (!sscanf(message, "%d", (int*)&header)) return MSG_UNKNOWN;
    return header;
}

Arguments parse_arguments(char* message) {
    Arguments args;
    char* token = strtok(message, " ");
    args.argc = 0;
    args.argv = NULL;

    while (token != NULL) {
        args.argc++;
        args.argv = realloc(args.argv, args.argc * sizeof(char*));
        ((char**)args.argv)[args.argc - 1] = token;
        token = strtok(NULL, " ");
    }
    return args;
}

int on_message(Codenames* codenames, TcpClient* client, char* message) {
    MessageType header = fetch_header(message);
    message += number_length((int)header) + 1; // Skip header et espace

    printf("[MSG] %d %s\n", header, message);

    Arguments args = parse_arguments(message);

    switch (header) {
        case MSG_UNKNOWN: 
            printf("Received unknown message header from client %d: \"%s\"\n", client->id, message);
            break;

        case MSG_CREATELOBBY: return request_create_lobby(codenames, client, message, args);
        case MSG_JOINLOBBY: return request_join_lobby(codenames, client, message, args);

        case MSG_STARTGAME: break; // Handle start game
        
        case MSG_REQUESTUUID: return request_uuid(codenames, client, message, args);
        
    }

    return EXIT_SUCCESS;
}

int on_leave(Codenames* codenames, TcpClient* client) {
    // Handle client disconnection
    printf("Client %d disconnected\n", client->id);

    Lobby* lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (lobby) {
        int id = lobby->id;
        /* S'il reste des joueurs dans le lobby, on ne le détruit pas et on passe l'owner à un des joueurs restants */
        if (lobby->nb_players > 1) {
            lobby->owner_id = lobby->users[0]->id;
        } else {
            destroy_lobby(codenames, lobby);
        }
        printf("Destroyed lobby %d owned by client %d\n", id, client->id);
    }

    return EXIT_SUCCESS;
}