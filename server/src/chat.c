#include "../lib/all.h"

int request_send_chat(Codenames* codenames, TcpClient* client, char* message, Arguments args) {
    if (args.argc < 2) {
        printf("Invalid chat message from client %d: \"%s\"\n", client->id, message);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "Invalid chat message format");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Vérifie que le client est bien dans un lobby
    Lobby* lobby = find_lobby_by_playerid(codenames->lobby, client->id);
    if (!lobby) {
        printf("Client %d is not in a lobby\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "You must be in a lobby to send chat messages");
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    if (!args.argv[0]) {
        printf("Sender user not found for client %d\n", client->id);
        char msg[64];
        format_to(msg, sizeof(msg), "%d %s", MSG_SERVER_ERROR, "User %s not found in lobby", (char*)args.argv[0]);
        tcp_send_to_client(codenames, client->id, msg);
        return EXIT_FAILURE;
    }

    // Diffuse le message de chat à tous les joueurs du lobby
    char msg[512];
    format_to(msg, sizeof(msg), "%d %s: %s", MSG_SENDCHAT, args.argv[0], args.argv[1]);
    for (int i = 0; i < lobby->nb_players; i++) {
        tcp_send_to_client(codenames, lobby->users[i]->id, msg);
    }

    return EXIT_SUCCESS;
} 