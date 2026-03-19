#ifndef MESSAGE_H
#define MESSAGE_H

#include "../lib/tcp.h"

/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_UNKNOWN message inconnu.
 * @param MSG_CREATELOBBY message de création de lobby.
 * @param MSG_JOINLOBBY message de rejoindre un lobby.
 * @param MSG_LEAVELOBBY message de quitter un lobby.
 * @param MSG_LOBBYCLOSED message de fermeture de lobby.
 * @param MSG_PLAYERJOINED message d'un joueur qui a rejoint le lobby.
 * @param MSG_CHOOSE_ROLE message de choix de rôle/équipe dans le lobby (pour informer les autres joueurs en temps réel du choix d'un joueur).
 * @param MSG_STARTGAME message de démarrage de partie. 
 * @param MSG_REQUESTUUID message de demande d'UUID (pour le client de demander son UUID au serveur après connexion, si besoin pour le protocole).
 */
typedef enum MessageType {
    MSG_UNKNOWN = -1,
    MSG_CREATELOBBY,
    MSG_JOINLOBBY,
    MSG_LEAVELOBBY,
    MSG_LOBBYCLOSED,
    MSG_PLAYERJOINED,
    MSG_CHOOSE_ROLE,
    MSG_STARTGAME,
    MSG_REQUESTUUID,
} MessageType;

/**
 * Structure pour stocker les arguments d'un message.
 * @param argc nombre d'arguments.
 * @param argv tableau de pointeurs vers les arguments.
 */
typedef struct Arguments {
    int argc;
    char** argv;
} Arguments;

/**
 * Extrait le type de message (HEADER) d'un message brut.
 * @param message message brut reçu du client.
 * @return le type de message extrait, ou MSG_UNKNOWN si le format est invalide.
 */
Arguments parse_arguments(char* message);

/* Forward declaration to avoid circular include with codenames.h */
typedef struct Codenames Codenames;

/**
 * Traite un message entrant côté serveur.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé le message.
 * @param message Message brut reçu du client.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int on_message(Codenames* codenames, TcpClient* client, char* message);

/**
 * Traite la déconnexion d'un client.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP qui s'est déconnecté.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int on_leave(Codenames* codenames, TcpClient* client);

#endif // MESSAGE_H