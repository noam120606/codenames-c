#ifndef LOBBY_H
#define LOBBY_H

#include "user.h"
#include "game.h"

#define MAX_LOBBIES 50
#define MAX_USERS 8 // Par game

#define LB_STATUS_WAITING 0
#define LB_STATUS_IN_GAME 1
typedef struct {
    int id;
    int status;
    User* users[MAX_USERS];
    int nb_players;
    Game* game;
} Lobby;

typedef struct {
    Lobby* lobbies[MAX_LOBBIES];
    int nb_lobbies;
} LobbyManager;

// Fonctions
LobbyManager* create_lobby_manager();
void destroy_lobby_manager(LobbyManager* manager);
Lobby* create_lobby(LobbyManager* manager);
void destroy_lobby(Lobby* lobby);

#endif // LOBBY_H