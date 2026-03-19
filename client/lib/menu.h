#ifndef MENU_H
#define MENU_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;
#include "../lib/button.h"

/**
 *  Initialise les ressources du menu.
 * @param context Contexte SDL.
 * @return Nombre d'erreurs survenues lors du chargement (0 si tout s'est bien passé, >0 sinon).
 */
int menu_init(AppContext* context);

/**
 * Affiche le menu principal.
 * @param context Contexte SDL.
 */
void menu_display(AppContext* context);

/**
 * Handle SDL events for the menu (inputs, buttons, etc.).
 * @return ButtonReturn (BTN_RET_QUIT to exit, BTN_RET_NONE otherwise).
 */
ButtonReturn menu_handle_event(AppContext* context, SDL_Event* e);

/** 
 * Libère les ressources du menu.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int menu_free();

#endif // MENU_H