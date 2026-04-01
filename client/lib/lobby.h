/**
 * @file lobby.h
 * @brief Gestion du lobby (salle d'attente avant la partie) côté client.
 */

#ifndef LOBBY_H
#define LOBBY_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;
typedef struct Window Window;

#include "../lib/button.h"
#include "../lib/user.h"
#include "../lib/chat.h"


#define MAX_LOBBIES 50
#define MAX_USERS 8

/**
 * Niveaux de difficulté du jeu.
 * @param WORDS_DIFFICULTY_EASY Difficulté facile (wordlist.txt).
 * @param WORDS_DIFFICULTY_HARD Difficulté difficile (wordlist_hard.txt).
 */
typedef enum WordsDifficulty {
    WORDS_DIFFICULTY_EASY = 0,
    WORDS_DIFFICULTY_HARD = 1
} WordsDifficulty;

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
 * @param code code d'accès au lobby (généré aléatoirement).
 * @param status état courant du lobby (LB_STATUS_*).
 * @param users tableau de pointeurs vers les joueurs présents.
 * @param nb_players nombre de joueurs actuellement connectés.
 * @param game partie associée au lobby (NULL si aucune partie).
 * @param chat historique du chat du lobby.
 * @param words_difficulty niveau de difficulté choisi pour la partie.
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
} Lobby;

/**
 * Initialise la structure d'un lobby.
 * @param lobby Pointeur vers le lobby à initialiser.
 * @param id Identifiant du lobby.
 * @param code Code d'accès au lobby.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int struct_lobby_init(Lobby* lobby, int id, const char* code);

/**
 * Initialise le lobby.
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
 * Gère les événements du lobby (boutons, windows, etc.).
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 */
void lobby_handle_event(AppContext* context, SDL_Event* event);

/**
 * Affiche le lobby.
 * @param context Contexte SDL.
 */
void lobby_display(AppContext* context);

/**
 * Affiche les joueurs dans les sections du lobby.
 * @param context Contexte SDL.
 * @param user Utilisateur dont l'icône doit être affichée.
 * @param nb_none Nombre de joueurs sans équipe.
 * @param i_none Indice du joueur sans équipe.
 * @param nb_red_spy Nombre de joueurs de l'équipe rouge (espions).
 * @param i_red_spy Indice du joueur espion rouge.
 * @param nb_red_agent Nombre de joueurs de l'équipe rouge (agents).
 * @param i_red_agent Indice du joueur agent rouge.
 * @param nb_blue_spy Nombre de joueurs de l'équipe bleue (espions).
 * @param i_blue_spy Indice du joueur espion bleu.
 * @param nb_blue_agent Nombre de joueurs de l'équipe bleue (agents).
 * @param i_blue_agent Indice du joueur agent bleu.
 */
void player_display(AppContext* context, User* user, int nb_none, int i_none, int nb_red_spy, int i_red_spy, int nb_red_agent, int i_red_agent, int nb_blue_spy, int i_blue_spy, int nb_blue_agent, int i_blue_agent);

/**
 * Positionne l'icône et le pseudo d'un joueur.
 * @param context Contexte SDL.
 * @param user Utilisateur dont l'icône doit être positionnée.
 * @param icon Texture de l'icône à afficher.
 * @param target_window Fenêtre cible de rendu (positionnement relatif).
 * @param nb_player Nombre de joueurs de ce type.
 * @param i_player Indice du joueur de ce type.
 * @param base_rel_x Position de base en X relative au centre de la fenêtre.
 * @param base_rel_y Position de base en Y relative au centre de la fenêtre.
 */
void player_icon_pos(AppContext* context, User* user, SDL_Texture* icon, Window* target_window, int nb_player, int i_player, int base_rel_x, int base_rel_y);

#endif // LOBBY_H