/**
 * @file menu.h
 * @brief Gestion du menu principal du jeu.
 */

#ifndef MENU_H
#define MENU_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../lib/button.h"

typedef struct AppContext AppContext;

/**
 * Initialise les ressources graphiques et UI du menu.
 * @param context Contexte applicatif.
 * @return 0 si tout est chargé correctement, sinon un compteur d'erreurs.
 */
int menu_init(AppContext* context);

/**
 * Affiche le menu principal.
 * @param context Contexte applicatif.
 */
void menu_display(AppContext* context);

/**
 * Gère les événements SDL du menu (inputs, boutons, tutoriel).
 * @param context Contexte applicatif.
 * @param event Événement SDL à traiter.
 * @return BTN_MENU_QUIT si l'utilisateur demande de quitter, BTN_NONE sinon.
 */
ButtonReturn menu_handle_event(AppContext* context, SDL_Event* event);

/**
 * Libère les ressources allouées par menu_init.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int menu_free();

#endif // MENU_H