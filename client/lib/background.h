#ifndef BACKGROUND_H
#define BACKGROUND_H

/**
 * Initialize the background.
 * @param context The SDL context containing the renderer.
 * @return The number of loading failures (0 if successful).
 */
int init_background(SDL_Context * context);

/**
 * Display the background.
 * @param context The SDL context containing the renderer.
 */
void display_background(SDL_Context * context);

/**
 * Destroy the background.
 */
void destroy_background();

#endif // BACKGROUND_H