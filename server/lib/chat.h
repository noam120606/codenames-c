/**
 * @file chat.h
 * @brief Gestion du système de chat en jeu côté serveur.
 */

#ifndef SERVER_CHAT_H
#define SERVER_CHAT_H

#include "../lib/list.h"

#define CHAT_MAX_MESSAGES 100

/**
 * Structure représentant un chat.
 * @param messages Liste chaînée des messages du chat.
 * @param max_messages Nombre maximal de messages conservés.
 */
typedef struct Chat {
    List* messages;
    int max_messages;
} Chat;

/**
 * Initialise un chat avec une liste vide.
 * @param chat Chat à initialiser.
 * @param max_messages Nombre maximal de messages conservés.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int chat_init(Chat* chat, int max_messages);

/**
 * Ajoute un message en fin de chat et supprime le plus ancien si la limite est atteinte.
 * @param chat Chat cible.
 * @param message Message à ajouter (copie en interne).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int chat_push(Chat* chat, const char* message);

/**
 * Retourne le message à un index donné.
 * @param chat Chat cible.
 * @param index Index (base 0).
 * @return Pointeur vers le message, ou NULL si index invalide.
 */
const char* chat_get(Chat* chat, int index);

/**
 * Retourne le nombre de messages stockés.
 * @param chat Chat cible.
 * @return Nombre de messages.
 */
int chat_size(Chat* chat);

/**
 * Libère tous les messages du chat.
 * @param chat Chat à vider.
 */
void chat_clear(Chat* chat);


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