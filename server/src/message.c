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

    // printf("[MSG] %d %s\n", header, message);

    Arguments args = parse_arguments(message);

    switch (header) {
        case MSG_UNKNOWN: 
            printf("Received unknown message header from client %d: \"%s\"\n", client->id, message);
            break;

        case MSG_CREATELOBBY: return request_create_lobby(codenames, client, message, args);
        case MSG_JOINLOBBY: return request_join_lobby(codenames, client, message, args);
        case MSG_LEAVELOBBY: return request_leave_lobby(codenames, client, message, args);
        case MSG_LOBBYCLOSED: break; // Server -> Client only
        case MSG_CHOOSE_ROLE: return request_choose_role(codenames, client, message, args);
        case MSG_STARTGAME: return request_start_game(codenames, client, message, args);
        case MSG_SUBMIT_HINT: return request_submit_hint(codenames, client, message, args);

        case MSG_SENDCHAT: return request_send_chat(codenames, client, message, args);

        case MSG_REQUESTUUID: return request_uuid(codenames, client, message, args);
        case MSG_COMPAREVERSION: return on_version_compare(codenames, client, message, args);
        case MSG_PING:
            if (args.argc >= 1) {
                char response[64];
                format_to(response, sizeof(response), "%d %s", MSG_PING, (char*)args.argv[0]);
                tcp_send_to_client(codenames, client->id, response);
            }
            return EXIT_SUCCESS;

        default:
            // Les autres types de message sont gérés ailleurs ou sont réservés aux clients.
            break;
    }

    return EXIT_SUCCESS;
}

int on_leave(Codenames* codenames, TcpClient* client) {
    // Handle client disconnection

    Lobby* owned_lobby = find_lobby_by_ownerid(codenames->lobby, client->id);
    if (owned_lobby) {
        int id = owned_lobby->id;

        /* S'il reste des joueurs dans le lobby, on ne le détruit pas et on passe l'owner à un des joueurs restants */
        if (owned_lobby->nb_players > 1) {
            owned_lobby->owner_id = owned_lobby->users[0]->id;
            printf("Client %d (%s) is now the owner of lobby %d\n", owned_lobby->owner_id, owned_lobby->users[0]->name, id);
        } else {
            destroy_lobby(codenames, owned_lobby);
            printf("Destroyed lobby %d owned by client %d\n", id, client->id);
        }

    }

    // Informer les autres joueurs du lobby qu'un joueur a quitté
    Lobby* player_lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (player_lobby) {
        char msg[64];
        format_to(msg, sizeof(msg), "%d %d", MSG_PLAYERLEFT, client->id);
        for (int i = 0; i < player_lobby->nb_players; i++) {
            if (player_lobby->users[i]->id != client->id) {
                tcp_send_to_client(codenames, player_lobby->users[i]->id, msg);
            }
        }
    }

    return EXIT_SUCCESS;
}