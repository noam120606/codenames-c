/**
 * @file lobby.h
 * @brief Gestion du lobby (salle d'attente avant la partie) côté client.
 */

#ifndef LOBBY_H
#define LOBBY_H

#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;

#include "../lib/chat.h"
#include "../lib/user.h"

#define MAX_LOBBIES 50
#define MAX_USERS 8

/**
 * Niveaux de difficulté des mots pour la génération de la grille.
 * @param WORDS_DIFFICULTY_NORMAL Difficulté normale (wordlist.txt).
 * @param WORDS_DIFFICULTY_HARD Difficulté difficile (wordlist_hard.txt).
 * @param WORDS_DIFFICULTY_INFO Difficulté informative (wordlist_info.txt).
 * @param WORDS_DIFFICULTY_FREAKY Difficulté déjantée (wordlist_freaky.txt).
 */
typedef enum WordsDifficulty {
    WORDS_DIFFICULTY_NORMAL,
    WORDS_DIFFICULTY_HARD,
    WORDS_DIFFICULTY_INFO,
    WORDS_DIFFICULTY_FREAKY
} WordsDifficulty;

/**
 * Nombre d'assassins disponibles pour la partie.
 * Conservé pour documenter les valeurs attendues côté UI/réseau.
 * @param NUMBER_OF_ASSASSINS_1 Un assassin.
 * @param NUMBER_OF_ASSASSINS_2 Deux assassins.
 * @param NUMBER_OF_ASSASSINS_3 Trois assassins.
 */
typedef enum NumberOfAssassins {
    NUMBER_OF_ASSASSINS_1,
    NUMBER_OF_ASSASSINS_2,
    NUMBER_OF_ASSASSINS_3,
} NumberOfAssassins;

/**
 * États possibles d'un lobby.
 * @param LB_STATUS_WAITING lobby en attente (pas encore en partie).
 * @param LB_STATUS_IN_GAME lobby actuellement en partie.
 */
typedef enum LobbyStatus {
    LB_STATUS_WAITING,
    LB_STATUS_IN_GAME
} LobbyStatus;

/**
 * Représente un lobby de jeu.
 * @param id identifiant unique du lobby.
 * @param owner_id identifiant du joueur propriétaire du lobby.
 * @param code code d'accès au lobby (5 caractères + '\0').
 * @param status état courant du lobby (LB_STATUS_*).
 * @param users tableau de pointeurs vers les joueurs présents.
 * @param nb_players nombre de joueurs actuellement connectés.
 * @param game partie associée au lobby (NULL si aucune partie).
 * @param chat historique du chat du lobby.
 * @param words_difficulty niveau de difficulté choisi pour la partie.
 * @param nb_assassins nombre d'assassins choisis pour la partie.
 */
typedef struct Lobby {
    int id;
    int owner_id;
    char code[6];
    LobbyStatus status;
    User* users[MAX_USERS];
    int nb_players;
    Game* game;
    Chat chat;
    WordsDifficulty words_difficulty;
    int nb_assassins;
} Lobby;

/**
 * Initialise/réinitialise la structure d'un lobby.
 * @param lobby Pointeur vers le lobby à initialiser.
 * @param id Identifiant du lobby.
 * @param code Code d'accès au lobby (chaîne vide possible).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int struct_lobby_init(Lobby* lobby, int id, const char* code);

/**
 * Initialise les ressources graphiques et UI du lobby.
 * @param context Contexte SDL.
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int lobby_init(AppContext* context);

/**
 * Libère les ressources utilisées par le lobby.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int lobby_free();

/**
 * Gère les événements du lobby (boutons, fenêtres, scroll, etc.).
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 */
void lobby_handle_event(AppContext* context, SDL_Event* event);

/**
 * Affiche l'interface complète du lobby.
 * @param context Contexte SDL.
 */
void lobby_display(AppContext* context);

/**
 * Cherche le nom d'un joueur dans un lobby par son identifiant.
 * @param lobby Pointeur vers le lobby.
 * @param id Identifiant du joueur recherché.
 * @return Pointeur vers le nom du joueur (ou NULL si introuvable).
 */
const char* find_player_by_id(const Lobby* lobby, int id);

#endif // LOBBY_H