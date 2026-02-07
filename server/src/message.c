#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/tcp.h"
#include "../lib/codenames.h"
#include "../lib/message.h"

MessageType fetch_header(const char* message) {
    // Extract the header from the message
    MessageType header;
    sscanf(message, "%d", (int*)&header);
    return header;
}

int on_message(Codenames* codenames, TcpClient* client, const char* message) {
    MessageType header = fetch_header(message);
    switch (header) {
        case MSG_CREATELOBBY:
            // Handle create lobby
            break;
        case MSG_JOINLOBBY:
            // Handle join lobby
            break;
        case MSG_STARTGAME:
            // Handle start game
            break;
        default:
            // Unknown message
            break;
    }
    return EXIT_SUCCESS;
}