#ifndef LOBBY_H
#define LOBBY_H

#include "../lib/user.h"
#include "../lib/game.h"
#include "../lib/message.h"
#include "../lib/list.h"

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
 * Conteneurs de rôles pour faciliter la gestion des rôles/équipes dans le lobby.
 * Chaque conteneur correspond à un rôle/équipe spécifique (ex: RED_AGENT, BLUE_SPY, etc.) et contient les utilisateurs qui ont choisi ce rôle/équipe.
 * @param users tableau de pointeurs vers les utilisateurs ayant choisi ce rôle/équipe.
 * @param count_users nombre d'utilisateurs dans ce conteneur.
 */
typedef struct {
    User* users[MAX_USERS]; // Les utilisateurs ayant choisi ce rôle/équipe (au maximum MAX_USERS pour ne pas avoir à gérer des soucis de places dans les conteneurs de rôles)
    int count_users;
} LobbyRoleContainers;

/**
 * Représente un lobby de jeu.
 * @param id identifiant unique du lobby.
 * @param owner_id identifiant du joueur propriétaire du lobby.
 * @param code code d'accès au lobby (généré aléatoirement).
 * @param status état courant du lobby (LB_STATUS_*).
 * @param users tableau de pointeurs vers les joueurs présents.
 * @param role_containers conteneurs de rôles pour faciliter la gestion des rôles/équipes dans le lobby.
 * @param nb_players nombre de joueurs actuellement connectés.
 * @param game partie associée au lobby (NULL si aucune partie).
 */
typedef struct {
    int id;
    int owner_id;
    char code[6];
    LobbyStatus status;
    User* users[MAX_USERS];
    LobbyRoleContainers role_containers[5]; // Les NONE, RED_AGENT, RED_SPY, BLUE_AGENT, BLUE_SPY
    int nb_players;
    Game* game;
} Lobby;

/** 
 * Gestionnaire de lobbies.
 * @param lobbies liste chaînée de pointeurs vers les lobbies actifs.
 */
typedef struct {
    List* lobbies;
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

/** Permet à un utilisateur de quitter un lobby.
 * @param lobby Lobby à quitter.
 * @param user Utilisateur qui souhaite quitter le lobby.
 * @return EXIT_SUCCESS si l'utilisateur a quitté avec succès, EXIT_FAILURE si une erreur est survenue (ex: utilisateur non trouvé dans le lobby).
 */
int leave_lobby(Lobby* lobby, User* user);

/** Trouve un lobby par l'identifiant de son propriétaire.
 * @param manager Gestionnaire de lobbies à rechercher.
 * @param owner_id Identifiant du propriétaire du lobby recherché.
 * @return Pointeur vers le Lobby trouvé, ou NULL si aucun lobby ne correspond.
 */
Lobby* find_lobby_by_ownerid(LobbyManager* manager, int owner_id);

/**
 * Trouve un lobby par l'identifiant d'un joueur.
 * @param manager Gestionnaire de lobbies à rechercher.
 * @param id Identifiant du joueur recherché.
 * @return Pointeur vers le Lobby trouvé, ou NULL si aucun lobby ne correspond.
 */
Lobby* find_lobby_by_playerid(LobbyManager* manager, int id);

/** Trouve un lobby par son identifiant.
 * @param manager Gestionnaire de lobbies à rechercher.
 * @param id Identifiant du lobby recherché.
 * @return Pointeur vers le Lobby trouvé, ou NULL si aucun lobby ne correspond.
 */
Lobby* find_lobby_by_code(LobbyManager* manager, const char* code);

/** Détruit un lobby et libère ses ressources.
 * @param codenames Contexte principal du serveur.
 * @param lobby Lobby à détruire.
 */
void destroy_lobby(Codenames* codenames, Lobby* lobby);

// interactions joueurs

/**
 * Gère la demande de création d'un lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client à l'origine de la demande.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS si la demande est traitée avec succès, EXIT_FAILURE sinon.
 */
int request_create_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Gère la demande de rejoindre un lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client à l'origine de la demande.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS si la demande est traitée avec succès, EXIT_FAILURE sinon.
 */
int request_join_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Gère la demande de quitter un lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client à l'origine de la demande.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS si la demande est traitée avec succès, EXIT_FAILURE sinon.
 */
int request_leave_lobby(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Gère la demande de choix de rôle/équipe dans le lobby.
 * @param codenames Contexte principal du serveur.
 * @param client Client à l'origine de la demande.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS si la demande est traitée avec succès, EXIT_FAILURE sinon.
 */
int request_choose_role(Codenames* codenames, TcpClient* client, char* message, Arguments args);

/**
 * Gère la demande d'UUID d'un client.
 * @param codenames Contexte principal du serveur.
 * @param client Client à l'origine de la demande.
 * @param message Message reçu du client.
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS si la demande est traitée avec succès, EXIT_FAILURE sinon.
 */
int request_uuid(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // LOBBY_H