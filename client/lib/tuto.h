/**
 * @file tuto.h
 * @brief Gestion du tutoriel en diaporama dans le menu principal.
 */

#ifndef TUTO_H
#define TUTO_H

#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;

/**
 * Initialise le tutoriel (chargement des images, création de la fenêtre et des boutons).
 * Si SEEN_TUTO vaut 0, le tutoriel est automatiquement ouvert.
 * @param context Contexte SDL.
 * @return EXIT_SUCCESS si initialisé, EXIT_FAILURE sinon.
 */
int tuto_init(AppContext* context);

/**
 * Ouvre le tutoriel manuellement depuis le menu.
 * Le diaporama repart de la première image.
 */
void tuto_open(void);

/**
 * Indique si le tutoriel est actuellement affiché.
 * @return 1 si actif, 0 sinon.
 */
int tuto_is_active(void);

/**
 * Gère les événements SDL du tutoriel (boutons précédent/suivant).
 * @param context Contexte SDL.
 * @param e Événement SDL.
 */
void tuto_handle_event(AppContext* context, SDL_Event* e);

/**
 * Affiche la fenêtre tutoriel, l'image courante et les boutons de navigation.
 * @param context Contexte SDL.
 */
void tuto_display(AppContext* context);

/**
 * Libère les ressources du tutoriel.
 * @return EXIT_SUCCESS.
 */
int tuto_free(void);

#endif // TUTO_H