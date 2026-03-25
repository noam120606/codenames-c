

#ifndef CLIENT_CHAT_H
#define CLIENT_CHAT_H

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

#endif // CLIENT_CHAT_H