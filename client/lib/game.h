#ifndef GAME_H
#define GAME_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"
#include "../lib/button.h"

/**
 * Représente un mot dans la grille de Codenames.
 *
 * @param word texte du mot (terminé par \0).
 * @param team équipe à laquelle le mot appartient (TEAM_*).
 * @param revealed 0 si caché, 1 si révélé.
 * @param gender genre du mot (0 pour masculin, 1 pour féminin).
 */
typedef struct {
    char word[32];
    Team team;
    int revealed;
    int gender;
} Card;


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