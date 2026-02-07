#ifndef CODENAMES_H
#define CODENAMES_H

#include "tcp.h"
#include "lobby.h"

/**
 * Structure principale du serveur de Codenames.
 * @param tcp gestionnaire du serveur TCP (sockets, clients, etc.).
 * @param lobby gestionnaire des lobbies (parties en attente, joueurs, etc.).
 */
typedef struct {
    TcpServer* tcp;
    LobbyManager* lobby;
} Codenames;

#endif // CODENAMES_H