/**
 * @file game.h
 * @brief Définitions et structures pour la gestion de la partie de Codenames côté client.
 */

#ifndef GAME_H
#define GAME_H

#define NB_WORDS 25
#define GAME_HINTBAR_TEXT_LEN 192
#define GAME_HINTBAR_FEEDBACK_INFO_MS 2500U
#define GAME_HINTBAR_FEEDBACK_SUCCESS_MS 3000U
#define GAME_HINTBAR_FEEDBACK_ERROR_MS 5000U

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;
typedef struct Card Card;
typedef struct Game Game;

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
 * Priorité des messages de la barre de titre de la fenêtre d'indice.
 * Une priorité plus élevée masque temporairement les priorités plus faibles.
 */
typedef enum GameHintBarPriority {
    GAME_HINTBAR_PRIORITY_CONTEXT = 1,
    GAME_HINTBAR_PRIORITY_INFO = 2,
    GAME_HINTBAR_PRIORITY_ERROR = 3
} GameHintBarPriority;

/**
 * Message affichable dans la barre de titre de la fenêtre d'indice.
 * expire_at_ms à 0 signifie "persistant".
 */
typedef struct GameHintBarMessage {
    char text[GAME_HINTBAR_TEXT_LEN];
    SDL_Color color;
    int priority;
    Uint32 expire_at_ms;
    int active;
} GameHintBarMessage;

/**
 * Etat complet de la barre de titre de la fenêtre d'indice.
 * context est persistant, feedback est temporaire.
 */
typedef struct GameHintBarState {
    GameHintBarMessage context;
    GameHintBarMessage feedback;
    char applied_text[GAME_HINTBAR_TEXT_LEN];
    SDL_Color applied_color;
    int applied_valid;
} GameHintBarState;

#include "../lib/history.h"

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
struct Game {
    Card* cards;
    int nb_words;
    GameState state;
    char current_hint[64];
    int current_hint_count;
    History red_history;
    History blue_history;
    Team winner;
    GameHintBarState hint_bar;
};

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
 * Définit le message persistant de contexte pour la barre de titre d'indice.
 * @param context Contexte de l'application.
 * @param text Texte à afficher.
 * @param color Couleur de la barre de titre.
 */
void game_hint_bar_set_context(AppContext* context, const char* text, SDL_Color color);

/**
 * Définit un message temporaire de feedback pour la barre de titre d'indice.
 * @param context Contexte de l'application.
 * @param text Texte à afficher.
 * @param color Couleur de la barre de titre.
 * @param priority Priorité du feedback.
 * @param duration_ms Durée d'affichage en millisecondes (0 = persistant jusqu'à remplacement explicite).
 */
void game_hint_bar_set_feedback(AppContext* context, const char* text, SDL_Color color, int priority, Uint32 duration_ms);

/**
 * Efface le feedback temporaire de la barre de titre d'indice.
 * @param context Contexte de l'application.
 */
void game_hint_bar_clear_feedback(AppContext* context);

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