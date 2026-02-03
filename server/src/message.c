#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/tcp.h"
#include "../lib/codenames.h"
#include "../lib/message.h"

int fetch_header(const char* message) {
    // Extract the header from the message
    int header;
    sscanf(message, "%d", &header);
    return header;
}

int on_message(Codenames* codenames, const char* message) {
    int header = fetch_header(message);
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
    return 0;
}