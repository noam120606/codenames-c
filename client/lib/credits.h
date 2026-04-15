/**
 * @file credits.h
 * @brief Gestion de la fenetre des credits du menu principal.
 */

#ifndef CREDITS_H
#define CREDITS_H

#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;

/**
 * Initialise la fenetre des credits (fenetre, bouton, chargement du texte).
 * @param context Contexte applicatif.
 * @return EXIT_SUCCESS si initialise, EXIT_FAILURE sinon.
 */
int credits_init(AppContext* context);

/**
 * Ouvre la fenetre des credits.
 */
void credits_open(void);

/**
 * Indique si la fenetre des credits est actuellement ouverte.
 * @return 1 si ouverte, 0 sinon.
 */
int credits_is_active(void);

/**
 * Gere les evenements SDL de la fenetre des credits.
 * @param context Contexte applicatif.
 * @param e Evenement SDL.
 */
void credits_handle_event(AppContext* context, SDL_Event* e);

/**
 * Affiche la fenetre des credits et le texte defilant.
 * @param context Contexte applicatif.
 */
void credits_display(AppContext* context);

/**
 * Libere les ressources allouees par le module credits.
 * @return EXIT_SUCCESS.
 */
int credits_free(void);

#endif // CREDITS_H
