#ifndef LOBBY_H
#define LOBBY_H

#include "user.h"
#include "game.h"

/* Forward declaration to avoid circular include with codenames.h */
typedef struct Codenames Codenames;

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
 * @param owner_id identifiant du joueur propriétaire du lobby.
 * @param status état courant du lobby (LB_STATUS_*).
 * @param users tableau de pointeurs vers les joueurs présents.
 * @param nb_players nombre de joueurs actuellement connectés.
 * @param game partie associée au lobby (NULL si aucune partie).
 */
typedef struct {
    int id;
    int owner_id;
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

/** Initialise le gestionnaire de lobbies.
 * @return EXIT_SUCCESS si OK, EXIT_FAILURE en cas d'erreur.
 */
int init_lobby_manager();

/**
 * Lit les noms d'utilisateur possibles depuis assets/usernames.txt et les stocke dans un tableau.
 * @return Tableau de chaînes de caractères contenant les noms d'utilisateur, ou NULL en cas d
 */
char** fetchUsernames();

/** Sélectionne un nom d'utilisateur aléatoire parmi ceux disponibles.
 * @return Chaîne de caractères contenant le nom d'utilisateur sélectionné, ou NULL en cas d'erreur.
 */
char* getRandomUsername();

/** Crée et initialise un gestionnaire de lobbies.
 * @param codenames Contexte principal du serveur, nécessaire pour certaines opérations (ex: fermer les connexions des joueurs lors de la destruction d'un lobby). 
 * @return Pointeur vers le LobbyManager, ou NULL en cas d'erreur.
 */
LobbyManager* create_lobby_manager();

/** Détruit un gestionnaire de lobbies et libère ses ressources.
 * @param codenames Contexte principal du serveur.
 * @param manager Gestionnaire à détruire.
 */
void destroy_lobby_manager(Codenames* codenames, LobbyManager* manager);

/** Crée un lobby et l'enregistre dans le gestionnaire.
 * @param manager Gestionnaire cible.
 * @return Pointeur vers le Lobby créé, ou NULL en cas d'erreur.
 */
Lobby* create_lobby(LobbyManager* manager);

/** Permet à un utilisateur de rejoindre un lobby.
 * @param lobby Lobby à rejoindre.
 * @param user Utilisateur qui souhaite rejoindre le lobby.
 * @return EXIT_SUCCESS si l'utilisateur a rejoint avec succès, EXIT_FAILURE si le lobby est plein.
 */
int join_lobby(Lobby* lobby, User* user);

/** Trouve un lobby par l'identifiant de son propriétaire.
 * @param manager Gestionnaire de lobbies à rechercher.
 * @param owner_id Identifiant du propriétaire du lobby recherché.
 * @return Pointeur vers le Lobby trouvé, ou NULL si aucun lobby ne correspond.
 */
Lobby* find_lobby_by_ownerid(LobbyManager* manager, int owner_id);

/** Détruit un lobby et libère ses ressources.
 * @param codenames Contexte principal du serveur.
 * @param lobby Lobby à détruire.
 */
void destroy_lobby(Codenames* codenames, Lobby* lobby);

#endif // LOBBY_H