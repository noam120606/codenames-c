#ifndef LOBBY_H
#define LOBBY_H

#include "user.h"
#include "game.h"

#define MAX_LOBBIES 50
#define MAX_USERS 8

/**
 * États possibles d'un lobby.
 * @param LB_STATUS_WAITING lobby en attente (pas encore en partie).
 * @param LB_STATUS_IN_GAME lobby actuellement en partie.
 */
typedef enum LobbyStatus {
    LB_STATUS_WAITING,
    LB_STATUS_IN_GAME
} LobbyStatus;

/**
 * Représente un lobby de jeu.
 * @param id identifiant unique du lobby.
 * @param status état courant du lobby (LB_STATUS_*).
 * @param users tableau de pointeurs vers les joueurs présents.
 * @param nb_players nombre de joueurs actuellement connectés.
 * @param game partie associée au lobby (NULL si aucune partie).
 */
typedef struct {
    int id;
    LobbyStatus status;
    User* users[MAX_USERS];
    int nb_players;
    Game* game;
} Lobby;

/** 
 * Gestionnaire de lobbies.
 * @param lobbies tableau de pointeurs vers les lobbies actifs.
 * @param nb_lobbies nombre de lobbies actuellement créés.
 */
typedef struct {
    Lobby* lobbies[MAX_LOBBIES];
    int nb_lobbies;
} LobbyManager;


/** Crée et initialise un gestionnaire de lobbies.
 * @return Pointeur vers le LobbyManager, ou NULL en cas d'erreur.
 */
LobbyManager* create_lobby_manager();

/** Détruit un gestionnaire de lobbies et libère ses ressources.
 * @param manager Gestionnaire à détruire.
 */
void destroy_lobby_manager(LobbyManager* manager);

/** Crée un lobby et l'enregistre dans le gestionnaire.
 * @param manager Gestionnaire cible.
 * @return Pointeur vers le Lobby créé, ou NULL en cas d'erreur.
 */
Lobby* create_lobby(LobbyManager* manager);

/** Détruit un lobby et libère ses ressources.
 * @param lobby Lobby à détruire.
 */
void destroy_lobby(Lobby* lobby);

#endif // LOBBY_H