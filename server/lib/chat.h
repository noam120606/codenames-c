

#ifndef SERVER_CHAT_H
#define SERVER_CHAT_H

/**
 * @param messages tableau dynamique de messages.
 * @param count nombre de messages dans le chat.
 * @param capacity capacité actuelle du tableau de messages.
 */
typedef struct Chat {
    char** messages;
    int count;
    int capacity;
} Chat;


/**
 * Traite une requête d'envoi de message de chat d'un client.
 * @param codenames Contexte principal du serveur.
 * @param client Client TCP ayant envoyé le message de chat.
 * @param message Nom du joueur + message brut reçu du client. 
 * @param args Arguments extraits du message.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int request_send_chat(Codenames* codenames, TcpClient* client, char* message, Arguments args);

#endif // SERVER_CHAT_H