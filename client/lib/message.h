#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_UNKNOWN message inconnu.
 * @param MSG_CREATELOBBY message de création de lobby.
 * @param MSG_JOINLOBBY message de rejoindre un lobby.
 * @param MSG_STARTGAME message de démarrage de partie. 
 * @param MSG_LOBBYCLOSED message de fermeture de lobby.
 */
typedef enum MessageType {
    MSG_UNKNOWN = -1,
    MSG_CREATELOBBY,
    MSG_JOINLOBBY,
    MSG_STARTGAME,
    MSG_LOBBYCLOSED
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
int on_message(SDL_Context* context, char* message);

#endif // MESSAGE_H