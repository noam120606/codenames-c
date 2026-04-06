/**
 * @file chat.h
 * @brief Gestion du système de chat en jeu côté client.
 */

#ifndef CLIENT_CHAT_H
#define CLIENT_CHAT_H

#include "../lib/list.h"

typedef struct AppContext AppContext;
typedef struct Window Window;
typedef struct Text Text;

#define CHAT_MAX_MESSAGES 100
#define CHAT_LINE_SIZE 256
#define CHAT_MAX_RENDER_LINES (CHAT_MAX_MESSAGES * 4)
#define CHAT_FONT_PATH_MAX 260

/**
 * Cache des lignes wrapees du chat pour eviter les recalculs a chaque frame.
 */
typedef struct ChatWrapCache {
    unsigned int source_revision;
    int max_text_width;
    int font_size;
    char font_path[CHAT_FONT_PATH_MAX];
    int total_lines;
    int is_valid;
    char lines[CHAT_MAX_RENDER_LINES][CHAT_LINE_SIZE];
} ChatWrapCache;

/**
 * Structure représentant un chat.
 * @param messages Liste chaînée des messages du chat.
 * @param max_messages Nombre maximal de messages conservés.
 * @param revision Revision incrementee quand le contenu du chat change.
 * @param wrap_cache Cache de wrapping pour le rendu.
 */
typedef struct Chat {
    List* messages;
    int max_messages;
    unsigned int revision;
    ChatWrapCache wrap_cache;
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
 * Callback de soumission du champ de chat.
 * Envoie le message au serveur si le texte n'est pas vide.
 * @param context Contexte applicatif.
 * @param text Texte saisi.
 */
void chat_submit_message(AppContext* context, const char* text);

/**
 * Rend les messages du chat dans une fenêtre scrollable avec retour à la ligne automatique.
 * @param context Contexte applicatif.
 * @param chat_window Fenêtre de chat cible.
 * @param chat_texts Tableau de Text* utilisé pour afficher les lignes visibles.
 * @param visible_lines Nombre de lignes visibles dans le tableau.
 */
void chat_render_messages(AppContext* context, Window* chat_window, Text** chat_texts, int visible_lines);

#endif // CLIENT_CHAT_H