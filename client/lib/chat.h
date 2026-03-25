

#ifndef CLIENT_CHAT_H
#define CLIENT_CHAT_H

#include "../lib/list.h"

#define CHAT_MAX_MESSAGES 100

/**
 * @param messages liste chainee des messages du chat.
 * @param max_messages nombre maximal de messages conserves.
 */
typedef struct Chat {
    List* messages;
    int max_messages;
} Chat;

/**
 * Initialise un chat avec une liste vide.
 * @param chat Chat a initialiser.
 * @param max_messages Nombre maximal de messages conserves.
 * @return EXIT_SUCCESS en cas de succes, EXIT_FAILURE sinon.
 */
int chat_init(Chat* chat, int max_messages);

/**
 * Ajoute un message en fin de chat et supprime le plus ancien si la limite est atteinte.
 * @param chat Chat cible.
 * @param message Message a ajouter (copie en interne).
 * @return EXIT_SUCCESS en cas de succes, EXIT_FAILURE sinon.
 */
int chat_push(Chat* chat, const char* message);

/**
 * Retourne le message a un index donne.
 * @param chat Chat cible.
 * @param index Index 0-based.
 * @return Pointeur vers le message, ou NULL si index invalide.
 */
const char* chat_get(Chat* chat, int index);

/**
 * Retourne le nombre de messages stockes.
 * @param chat Chat cible.
 * @return Nombre de messages.
 */
int chat_size(Chat* chat);

/**
 * Libere tous les messages du chat.
 * @param chat Chat a vider.
 */
void chat_clear(Chat* chat);

#endif // CLIENT_CHAT_H