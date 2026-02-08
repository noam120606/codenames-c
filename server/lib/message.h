#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_CREATELOBBY message de création de lobby.
 * @param MSG_JOINLOBBY message de rejoindre un lobby.
 * @param MSG_STARTGAME message de démarrage de partie. 
 */
typedef enum MessageType {
    MSG_UNKNOWN = -1,
    MSG_CREATELOBBY,
    MSG_JOINLOBBY,
    MSG_STARTGAME
} MessageType;

#include "codenames.h"

/**
 * Traite un message entrant côté serveur.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé le message.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int on_message(Codenames* codenames, TcpClient* client, char* message);

#endif // MESSAGE_H