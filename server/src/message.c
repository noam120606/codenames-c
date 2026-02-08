#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/tcp.h"
#include "../lib/codenames.h"
#include "../lib/message.h"
#include "../lib/utils.h"

MessageType fetch_header(char* message) {
    MessageType header;
    if (!sscanf(message, "%d", (int*)&header)) return MSG_UNKNOWN;
    return header;
}

int on_message(Codenames* codenames, TcpClient* client, char* message) {
    MessageType header = fetch_header(message);
    message += number_length((int)header) + 1; // Skip header et espace

    switch (header) {
        case MSG_UNKNOWN: 
            printf("Received unknown message header from client %d: \"%s\"\n", client->id, message);
            break;
        case MSG_CREATELOBBY:
            // Handle create lobby
            printf("%s", message);
            break;
        case MSG_JOINLOBBY:
            // Handle join lobby
            break;
        case MSG_STARTGAME:
            // Handle start game
            break;
    }
    return EXIT_SUCCESS;
}