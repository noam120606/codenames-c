#include "../lib/all.h"

MessageType fetch_header(char* message) {
    MessageType header;
    if (!sscanf(message, "%d", (int*)&header)) return MSG_UNKNOWN;
    return header;
}

Arguments parse_arguments(char* message) {
    Arguments args = {0};
    char* token = strtok(message, " ");

    while (token != NULL) {
        char** resized_argv = realloc(args.argv, (args.argc + 1) * sizeof(char*));
        if (!resized_argv) {
            free(args.argv);
            args.argv = NULL;
            args.argc = 0;
            break;
        }

        args.argv = resized_argv;
        args.argv[args.argc] = token;
        args.argc++;
        token = strtok(NULL, " ");
    }

    return args;
}

int on_message(Codenames* codenames, TcpClient* client, char* message) {
    MessageType header = fetch_header(message);
    message += number_length((int)header) + 1; // Skip header et espace

    // printf("[MSG] %d %s\n", header, message);

    Arguments args = parse_arguments(message);
    int status = EXIT_SUCCESS;

    switch (header) {
        case MSG_UNKNOWN: 
            printf("Received unknown message header from client %d: \"%s\"\n", client->id, message);
            break;

        case MSG_CREATELOBBY:
            status = request_create_lobby(codenames, client, message, args);
            break;
        case MSG_JOINLOBBY:
            status = request_join_lobby(codenames, client, message, args);
            break;
        case MSG_LEAVELOBBY:
            status = request_leave_lobby(codenames, client, message, args);
            break;
        case MSG_LOBBYCLOSED: break; // Server -> Client only
        case MSG_CHOOSE_ROLE:
            status = request_choose_role(codenames, client, message, args);
            break;
        case MSG_STARTGAME:
            status = request_start_game(codenames, client, message, args);
            break;
        case MSG_SUBMIT_HINT:
            status = request_submit_hint(codenames, client, message, args);
            break;
        case MSG_PREGUESS:
            status = request_preguess(codenames, client, message, args);
            break;
        case MSG_GUESS_CARD:
            status = request_guess_card(codenames, client, message, args);
            break;
        case MSG_SET_WORDS_DIFFICULTY:
            status = request_set_words_difficulty(codenames, client, message, args);
            break;
        case MSG_SET_NB_ASSASSINS:
            status = request_set_nb_assassins(codenames, client, message, args);
            break;

        case MSG_SENDCHAT:
            status = request_send_chat(codenames, client, message, args);
            break;

        case MSG_REQUESTUUID:
            status = request_uuid(codenames, client, message, args);
            break;
        case MSG_COMPAREVERSION:
            status = on_version_compare(codenames, client, message, args);
            break;
        case MSG_PING:
            if (args.argc >= 1) {
                char response[64];
                format_to(response, sizeof(response), "%d %s", MSG_PING, args.argv[0]);
                tcp_send_to_client(codenames, client->id, response);
            }
            status = EXIT_SUCCESS;
            break;

        default:
            // Les autres types de message sont gérés ailleurs ou sont réservés aux clients.
            break;
    }

    free(args.argv);
    return status;
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