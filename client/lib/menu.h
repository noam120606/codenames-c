#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"

/**
 * Actions possibles dans le menu principal.
 * @param MENU_ERROR une erreur est survenue.
 * @param MENU_ACTION_NONE aucune action choisie.
 * @param MENU_ACTION_QUIT quitter le jeu.
 * @param MENU_ACTION_CREATE_LOBBY créer un lobby.
 * @param MENU_ACTION_JOIN_LOBBY rejoindre un lobby.
 */
typedef enum MenuAction {
    MENU_ERROR = -1,
    MENU_ACTION_NONE = 0,
    MENU_ACTION_QUIT,
    MENU_ACTION_CREATE_LOBBY,
    MENU_ACTION_JOIN_LOBBY
} MenuAction;

/**
 *  Initialise les ressources du menu.
 * @param context Contexte SDL.
 * @return Nombre d'erreurs survenues lors du chargement (0 si tout s'est bien passé, >0 sinon).
 */
int menu_init(SDL_Context context);

/**
 * Affiche le menu principal.
 * @param context Contexte SDL.
 * @return Action choisie par l'utilisateur dans le menu.
 */
MenuAction menu_display(SDL_Context context);

/** 
 * Libère les ressources du menu.
 * @param context Contexte SDL.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int menu_free(SDL_Context context);

#endif // MENU_H