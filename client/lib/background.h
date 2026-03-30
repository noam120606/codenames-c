/**
 * @file background.h
 * @brief Gestion de l'arrière-plan animé du client.
 */

#ifndef BACKGROUND_H
#define BACKGROUND_H

/**
 * Initialise l'arrière-plan.
 * @param context Contexte SDL contenant le renderer.
 * @return Nombre d'échecs de chargement (0 si succès).
 */
int init_background(AppContext * context);

/**
 * Affiche l'arrière-plan.
 * @param context Contexte SDL contenant le renderer.
 */
void display_background(AppContext * context);

/**
 * Gère un événement SDL pour l'arrière-plan (le clic change le symbole de la tuile cliquée).
 * @param context Contexte SDL.
 * @param e Événement SDL à traiter.
 */
void background_handle_event(AppContext* context, SDL_Event* e);

/**
 * Détruit l'arrière-plan et libère les ressources.
 * @return 0 en cas de succès.
 */
int destroy_background();

/**
 * Définit la couleur de l'arrière-plan.
 * @param r Composante rouge (0-255).
 * @param g Composante verte (0-255).
 * @param b Composante bleue (0-255).
 */
void background_set_color(Uint8 r, Uint8 g, Uint8 b);

/**
 * Récupère la couleur actuelle de l'arrière-plan.
 * @return Couleur SDL_Color de l'arrière-plan.
 */
SDL_Color background_get_color(void);

#endif // BACKGROUND_H