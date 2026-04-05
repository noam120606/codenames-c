/**
 * @file game.h
 * @brief Définitions et structures pour la gestion de la partie de Codenames côté client.
 */

#ifndef GAME_H
#define GAME_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;
typedef struct Card Card;

#include "../lib/button.h"

#define NB_WORDS 25

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
 * @param GAMESTATE_TURN_RED_SPY tour de l'espion rouge.
 * @param GAMESTATE_TURN_RED_AGENT tour de l'agent rouge.
 * @param GAMESTATE_TURN_BLUE_SPY tour de l'espion bleu.
 * @param GAMESTATE_TURN_BLUE_AGENT tour de l'agent bleu.
 * @param GAMESTATE_ENDED partie terminée.
 */
typedef enum GameState {
    GAMESTATE_WAITING,
    GAMESTATE_TURN_RED_SPY,
    GAMESTATE_TURN_RED_AGENT,
    GAMESTATE_TURN_BLUE_SPY,
    GAMESTATE_TURN_BLUE_AGENT,
    GAMESTATE_ENDED
} GameState;

/**
 * Représente un tour de jeu pour une équipe, avec les mots révélés à ce tour et le nom du joueur de l'équipe qui a joué.
 * @param spy_name Le nom du joueur qui a soumis l'indice.
 * @param hint_count Le nombre de mots associés à l'indice donné par l'espion pour ce tour (0 si c'est un tour d'agent).
 * @param agent_name Le nom du joueur qui a révélé le(s) mot(s) à ce tour (vide si c'est un tour d'espion).
 * @param hint L'indice donné par l'espion pour ce tour (vide si c'est un tour d'agent).
 * @param revealed_words Les mots révélés à ce tour (tableau de chaînes de caractères, taille NB_WORDS).
 */
typedef struct Turn {
    char spy_name[32]; // Le nom du joueur qui a soumis l'indice
    int hint_count; // Le nombre de mots associés à l'indice donné par l'espion pour ce tour (0 si c'est un tour d'agent)
    char agent_name[32]; // Le nom du joueur qui a révélé le(s) mot(s) à ce tour (vide si c'est un tour d'espion)
    char hint[64]; // L'indice donné par l'espion pour ce tour (vide si c'est un tour d'agent)
    char revealed_words[NB_WORDS][64]; // Les mots révélés à ce tour
} Turn;

/**
 * L'historique contient tous les tours de jeu d'une équipe, avec les mots révélés à chaque tour, le nom du joueur qui a soumis l'indice, l'indice et le nom du joueur qui a révélé le(s) mot(s).
 * Cela permet d'afficher les infos dans l'interface pour suivre l'évolution de la partie.
 * @param turns Tableau de tours de jeu (taille NB_WORDS, car on peut avoir au maximum NB_WORDS tours dans le cas où chaque tour révèle un mot).
 * @param turn_count Le nombre de tours joués jusqu'à présent.
 */
typedef struct History {
    Turn turns[NB_WORDS]; // On peut avoir au maximum NB_WORDS tours (dans le cas où chaque tour révèle un mot)
    int turn_count; // Le nombre de tours joués jusqu'à présent
} History;

/**
 * Représente une partie de Codenames.
 * @param cards tableau dynamique de Card (taille nb_words).
 * @param nb_words nombre de mots dans la grille.
 * @param state état courant de la partie (GAMESTATE_*).
 * @param current_hint mot indice actuel donné par l'espion.
 * @param current_hint_count nombre de mots associés à l'indice actuel.
 * @param red_history historique des mots révélés par l'équipe rouge (contient les infos sur les tours de jeu).
 * @param blue_history historique des mots révélés par l'équipe bleue (contient les infos sur les tours de jeu).
 * @param winner équipe gagnante (TEAM_RED, TEAM_BLUE ou TEAM_NONE si pas encore déterminée).
 */
typedef struct {
    Card* cards;
    int nb_words;
    GameState state;
    char current_hint[64];
    int current_hint_count;
    History red_history;
    History blue_history;
    Team winner;
} Game;

/**
 * Vérifie si c'est le tour du joueur.
 * @param context Contexte de l'application.
 * @return 1 si c'est le tour du joueur, 0 sinon.
 */
int my_turn(AppContext* context);

/**
 * Libère les ressources utilisées par la structure de jeu.
 * @param context Contexte de l'application contenant la structure de jeu à libérer.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE si le contexte ou la structure de jeu est invalide.
 */
int game_struct_free(AppContext* context);

/**
 * Gère les événements du menu.
 * @param context Contexte SDL.
 * @param e Événement SDL à traiter.
 */
void game_handle_event(AppContext* context, SDL_Event* e);

/**
 * Initialise le jeu.
 * @param context Contexte SDL.
 * @return 0 en cas de succès, le nombre d'erreur sinon.
 */
int game_init(AppContext * context);

/**
 * Libère les ressources utilisées par le jeu.
 * @return 0 en cas de succès, le nombre d'erreur sinon.
 */
int game_free();

/**
 * Gère les événements du jeu.
 * @param context Contexte SDL.
 * @param event Événement SDL à traiter.
 */
void game_handle_event(AppContext * context, SDL_Event * event);

/**
 * Affiche le jeu.
 * @param context Contexte SDL.
 */
void game_display(AppContext * context);

/**
 * Rendu des cartes du jeu.
 * @param context Contexte SDL.
 */
void game_render_cards(AppContext * context);


#endif // GAME_H