#ifndef MENU_H
#define MENU_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"

/**
 *  Initialise les ressources du menu.
 * @param context Contexte SDL.
 * @return Nombre d'erreurs survenues lors du chargement (0 si tout s'est bien passé, >0 sinon).
 */
int menu_init(SDL_Context * context);

/**
 * Affiche le menu principal.
 * @param context Contexte SDL.
 */
void menu_display(SDL_Context * context);

/** 
 * Libère les ressources du menu.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int menu_free();

#endif // MENU_H