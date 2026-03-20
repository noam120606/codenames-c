#ifndef LOBBY_H
#define LOBBY_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;
#include "../lib/button.h"
#include "../lib/user.h"


#define MAX_LOBBIES 50
#define MAX_USERS 8

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
 */
typedef struct Lobby {
    int id;
    int owner_id;
    char code[6];
    LobbyStatus status;
    User* users[MAX_USERS];
    int nb_players;
    Game* game;
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
 */
int lobby_free();

/**
 * Gère les événements du lobby (boutons, etc.).
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 * @return ButtonReturn (BTN_RET_QUIT to exit, BTN_RET_NONE otherwise).
 */
ButtonReturn lobby_handle_event(AppContext* context, SDL_Event* event);

/**
 * Affiche le lobby.
 * @param context Contexte SDL.
 */
void lobby_display(AppContext* context);

/**
 * Affiche les icones des joueurs.
 * @param context Contexte SDL.
 * @param user Utilisateur dont l'icône doit être affichée.
 * @param nb_none Nombre de joueurs sans équipe.
 * @param nb_red_spy Nombre de joueurs de l'équipe rouge (espions).
 * @param nb_red_agent Nombre de joueurs de l'équipe rouge (agents).
 * @param nb_blue_spy Nombre de joueurs de l'équipe bleue (espions).
 * @param nb_blue_agent Nombre de joueurs de l'équipe bleue (agents).
 */
void player_icon_display(AppContext* context, User* user, int nb_none, int nb_red_spy, int nb_red_agent, int nb_blue_spy, int nb_blue_agent);

#endif // LOBBY_H