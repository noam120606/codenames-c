#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct AppContext AppContext;

/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_UNKNOWN message inconnu.
 * @param MSG_CREATELOBBY message de création de lobby.
 * @param MSG_JOINLOBBY message de rejoindre un lobby.
 * @param MSG_LEAVELOBBY message de quitter un lobby.
 * @param MSG_LOBBYCLOSED message de fermeture de lobby.
 * @param MSG_PLAYERJOINED message d'un joueur qui a rejoint le lobby.
 * @param MSG_PLAYERLEFT message d'un joueur qui a quitté le lobby.
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
    MSG_PLAYERLEFT,
    MSG_CHOOSE_ROLE,
    MSG_STARTGAME,
    MSG_REQUESTUUID
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

/**
 * Traite un message reçu du serveur.
 * @param context contexte SDL du client, nécessaire pour certaines opérations (ex: mise à jour de l'interface).
 * @param message message brut reçu du serveur.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int on_message(AppContext* context, char* message);

#endif // MESSAGE_H