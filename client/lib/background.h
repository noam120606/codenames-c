#ifndef BACKGROUND_H
#define BACKGROUND_H

/**
 * Initialize the background.
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

#endif // BACKGROUND_H