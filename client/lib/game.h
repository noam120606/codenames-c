#ifndef GAME_H
#define GAME_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct SDL_Context SDL_Context;
#include "../lib/button.h"

/**
 * TEAM est utilisé à la fois pour catégoriser les mots dans la grille et pour assigner les joueurs à une équipe.
 * Catégories de mots dans la grille de Codenames.
 * Les mots sont classés en 4 catégories :
 * @param TEAM_NONE mot neutre (aucune équipe).
 * @param TEAM_RED mot appartenant à l'équipe rouge.
 * @param TEAM_BLUE mot appartenant à l'équipe bleue.
 * @param TEAM_BLACK mot assassin (met fin à la partie si révélé).
 */
typedef enum Team {
    TEAM_NONE,
    TEAM_RED,
    TEAM_BLUE,
    TEAM_BLACK
} Team;

/**
 * Représente un mot dans la grille de Codenames.
 *
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param revealed 0 si caché, 1 si révélé.
 */
typedef struct {
    char word[32];
    Team team;
    int revealed;
} Word;

/**
 * État de l'application.
 * @param APP_STATE_MENU État du menu principal.
 * @param APP_STATE_LOBBY État du lobby.
 * @param APP_STATE_PLAYING État de la partie en cours.
 * @param APP_STATE_PAUSED État de la partie en pause.
 */
typedef enum AppState {
    APP_STATE_MENU,
    APP_STATE_LOBBY,
    APP_STATE_PLAYING,
    APP_STATE_PAUSED
} AppState;

/**
 * États possibles d'une partie.
 * @param GAMESTATE_WAITING en attente de joueurs / démarrage.
 * @param GAMESTATE_TURN_RED tour de l'équipe rouge.
 * @param GAMESTATE_TURN_BLUE tour de l'équipe bleue.
 * @param GAMESTATE_ENDED partie terminée.
 */
typedef enum GameState {
    GAMESTATE_WAITING,
    GAMESTATE_TURN_RED,
    GAMESTATE_TURN_BLUE,
    GAMESTATE_ENDED
} GameState;

/**
 * Représente une partie de Codenames.
 * @param id identifiant unique de la partie.
 * @param words tableau dynamique de Word (taille nb_words).
 * @param nb_words nombre de mots dans la grille.
 * @param state état courant de la partie (GAMESTATE_*).
 */
typedef struct {
    int id;
    Word* words;
    int nb_words;
    GameState state;
} Game;

/**
 * Représente un mot dans la grille de Codenames.
 *
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param revealed 0 si caché, 1 si révélé.
 * @param gender genre du mot (0 pour masculin, 1 pour féminin).
 */
typedef struct Card {
    char word[32];
    Team team;
    int revealed;
    int gender;
} Card;

/**
 * Gère les événements du menu.
 * @param context Contexte SDL.
 * @param e Événement SDL à traiter.
 */
void game_handle_event(SDL_Context* context, SDL_Event* e);

/**
 * Initialise le jeu.
 * @param context Contexte SDL.
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int game_init(SDL_Context * context);

/**
 * Libère les ressources utilisées par le jeu.
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int game_free();

/**
 * Gère les événements du jeu.
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 */
void game_handle_event(SDL_Context * context, SDL_Event * event);

/**
 * Affiche le jeu.
 * @param context Contexte SDL.
 */
void game_display(SDL_Context * context);


#endif // GAME_H