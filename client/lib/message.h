/**
 * @file message.h
 * @brief Définition des types de messages et traitement des messages réseau côté client.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct AppContext AppContext;

/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_SERVER_ERROR message d'erreur envoyé par le serveur en cas de problème (ex: format de message invalide, action interdite, etc.). Le message d'erreur est suivi d'une description de l'erreur pour aider au debug. 
 * @param MSG_UNKNOWN message inconnu.
 * @param MSG_INFO message d'information (pour des messages non critiques ou des mises à jour de statut).
 * @param MSG_CREATELOBBY message de création de lobby.
 * @param MSG_JOINLOBBY message de rejoindre un lobby.
 * @param MSG_LEAVELOBBY message de quitter un lobby.
 * @param MSG_LOBBYCLOSED message de fermeture de lobby.
 * @param MSG_PLAYERJOINED message d'un joueur qui a rejoint le lobby.
 * @param MSG_PLAYERLEFT message d'un joueur qui a quitté le lobby.
 * @param MSG_CHOOSE_ROLE message de choix de rôle/équipe dans le lobby (pour informer les autres joueurs en temps réel du choix d'un joueur).
 * @param MSG_STARTGAME message de démarrage de partie. 
 * @param MSG_WORDDATA message de données de mots pour la partie (envoyé par le serveur au démarrage de la partie, contient la liste des mots et leur couleur). 
 * @param MSG_SUBMIT_HINT message de soumission du mot indice et du nombre de mots indices.
 * @param MSG_SENDCHAT message d'envoi de message dans le chat.
 * @param MSG_REQUESTUUID message de demande d'UUID (pour le client de demander son UUID au serveur après connexion, si besoin pour le protocole).
 * @param MSG_COMPAREVERSION message de comparaison de version (pour le client de comparer sa version avec celle du serveur).
 * @param MSG_PING message d'echo client/serveur pour mesurer le ping applicatif en temps réel.
 */
typedef enum MessageType {
    MSG_SERVER_ERROR = -2,
    MSG_UNKNOWN = -1,
    MSG_INFO,
    MSG_CREATELOBBY,
    MSG_JOINLOBBY,
    MSG_LEAVELOBBY,
    MSG_LOBBYCLOSED,
    MSG_PLAYERJOINED,
    MSG_PLAYERLEFT,
    MSG_CHOOSE_ROLE,
    MSG_STARTGAME,
    MSG_WORDDATA,
    MSG_SUBMIT_HINT,
    MSG_SENDCHAT,
    MSG_REQUESTUUID,
    MSG_COMPAREVERSION,
    MSG_PING,
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
 * Extrait les arguments d'un message brut.
 * @param message Message brut reçu du client.
 * @return Structure Arguments contenant les arguments extraits.
 */
Arguments parse_arguments(char* message);

/**
 * Traite un message reçu du serveur.
 * @param context Contexte SDL du client, nécessaire pour certaines opérations (ex: mise à jour de l'interface).
 * @param message Message brut reçu du serveur.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int on_message(AppContext* context, char* message);

#endif // MESSAGE_H