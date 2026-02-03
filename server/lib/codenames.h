#ifndef CODENAMES_H
#define CODENAMES_H

#include "tcp.h"
#include "lobby.h"

typedef struct {
    TcpServer* tcp;
    Lobby* lobby;
} Codenames;

#endif // CODENAMES_H