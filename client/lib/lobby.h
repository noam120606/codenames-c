#ifndef LOBBY_H
#define LOBBY_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#include "../lib/sdl.h"
#include "../lib/button.h"

/**
 * Initialise le lobby.
 * @param context Contexte SDL.
 * @return 0 en cas de succès, un code d'erreur sinon.
 */
int lobby_init(SDL_Context* context);

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
ButtonReturn lobby_handle_event(SDL_Context* context, SDL_Event* event);

/**
 * Affiche le lobby.
 * @param context Contexte SDL.
 */
void lobby_display(SDL_Context* context);

#endif // LOBBY_H