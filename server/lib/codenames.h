#ifndef CODENAMES_H
#define CODENAMES_H

#include "tcp.h"
#include "lobby.h"

typedef struct {
    TcpServer* tcp;
    LobbyManager* lobby;
} Codenames;

#endif // CODENAMES_H