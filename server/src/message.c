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

        case MSG_CREATELOBBY: {
            // Handle create lobby
            if (args.argc < 1) {
                printf("Invalid create lobby message from client %d: \"%s\"\n", client->id, message);
                break;
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
            break;
        }

        case MSG_JOINLOBBY: {
            // Handle join lobby
            if (args.argc < 2) {
                printf("Invalid join lobby message from client %d: \"%s\"\n", client->id, message);
                break;
            }

            Lobby* lobby = find_lobby_by_code(codenames->lobby, args.argv[0]);
            if (!lobby) {
                printf("Lobby not found for client %d\n", client->id);
                break;
            }

            User* user = create_user(client->id, args.argv[1], client->socket);
            if (!user) {
                printf("Failed to create user for client %d\n", client->id);
                break;
            }

            if (join_lobby(lobby, user) != EXIT_SUCCESS) {
                printf("Failed to join lobby %d for client %d\n", lobby->id, client->id);
                destroy_user(user);
                break;
            }

            printf("Client %d joined lobby %d\n", client->id, lobby->id);

            break;
        }

        case MSG_STARTGAME: {
            // Handle start game
            break;
        }

        case MSG_REQUESTUUID: {
            // Générer un UUID unique et l'envoyer au client
            char* uuid = generate_uuid("data/uuids");
            if (uuid == NULL) {
                printf("Failed to generate UUID for client %d\n", client->id);
                return EXIT_FAILURE;
            }
            char reponse[128];
            format_to(reponse, sizeof(reponse), "%d %s", MSG_REQUESTUUID, uuid);
            tcp_send_to_client(codenames, client->id, reponse);
            printf("Generated UUID %s for client %d\n", uuid, client->id);
            free(uuid);
            break;
        }
    }
    return EXIT_SUCCESS;
}

int on_leave(Codenames* codenames, TcpClient* client) {
    // Handle client disconnection
    printf("Client %d disconnected\n", client->id);

    Lobby* lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (lobby) {
        int id = lobby->id;
        destroy_lobby(codenames, lobby);
        printf("Destroyed lobby %d owned by client %d\n", id, client->id);
    }

    return EXIT_SUCCESS;
}