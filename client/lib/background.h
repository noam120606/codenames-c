/**
 * @file background.h
 * @brief Gestion de l'arrière-plan animé du client.
 */

#ifndef BACKGROUND_H
#define BACKGROUND_H

/**
 * Initialise l'arrière-plan.
 * @param context The SDL context containing the renderer.
 * @return The number of loading failures (0 if successful).
 */
int init_background(AppContext * context);

/**
 * Display the background.
 * @param context The SDL context containing the renderer.
 */
void display_background(AppContext * context);

/**
 * Handle a SDL event for the background (click cycles the symbol of the clicked tile).
 * @param context The SDL context.
 * @param e       The SDL event to process.
 */
void background_handle_event(AppContext* context, SDL_Event* e);

/**
 * Destroy the background.
 */
int destroy_background();

/**
 * Set the background color.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void background_set_color(Uint8 r, Uint8 g, Uint8 b);

/**
 * Get the current background color.
 * @return The current SDL_Color of the background.
 */
SDL_Color background_get_color(void);

#endif // BACKGROUND_H