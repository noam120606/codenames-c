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
 * Met à jour la progression de chargement de l'intro de lancement (0.0 à 1.0).
 * @param progress Progression réelle du chargement.
 */
void menu_set_startup_loading_progress(float progress);

/**
 * Notifie le menu que le chargement des ressources de lancement est terminé.
 */
void menu_mark_startup_loading_complete();

/**
 * Demande de passer l'animation d'ouverture et d'aller directement
 * à la transition (bounce + fondu du background).
 */
void menu_request_startup_skip();

/**
 * Indique si le fond animé doit être rendu derrière le menu.
 * @return 1 si le fond doit être affiché, 0 sinon.
 */
int menu_should_render_background();

/**
 * Indique si l'animation d'introduction de lancement est terminée.
 * @return 1 si l'intro est terminée, 0 sinon.
 */
int menu_is_startup_animation_complete();

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