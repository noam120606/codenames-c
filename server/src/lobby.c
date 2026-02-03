#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/lobby.h"
#include "../lib/user.h"

LobbyManager* create_lobby_manager() {
    LobbyManager* manager = malloc(sizeof(LobbyManager));
    manager->nb_lobbies = 0;
    return manager;
}

void destroy_lobby_manager(LobbyManager* manager) {
    if (manager) {
        for (int i = 0; i < manager->nb_lobbies; i++) {
            destroy_lobby(manager->lobbies[i]);
        }
        free(manager);
    }
}

Lobby* create_lobby(LobbyManager* manager) {
    if (manager->nb_lobbies >= MAX_LOBBIES) {
        return NULL;
    }
    Lobby* lobby = malloc(sizeof(Lobby));
    lobby->id = manager->nb_lobbies;
    lobby->status = LB_STATUS_WAITING;
    lobby->nb_players = 0;
    lobby->game = NULL;
    manager->lobbies[manager->nb_lobbies++] = lobby;
    return lobby;
}

void destroy_lobby(Lobby* lobby) {
    if (lobby) {
        for (int i = 0; i < lobby->nb_players; i++) {
            destroy_user(lobby->users[i]);
        }
        free(lobby);
    }
}