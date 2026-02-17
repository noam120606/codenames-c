/**
 * Types de messages harmonisé client/serveur (aussi appelé HEADER) 
 * @param MSG_UNKNOWN message inconnu.
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