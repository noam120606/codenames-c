#ifndef CROSSFADER_H
#define CROSSFADER_H

#include "../lib/sdl.h"

#define MAX_CROSSFADERS 32

typedef struct {
    int id;
    SDL_Rect rect; /* full track rect */
    int min;
    int max;
    int value;
    int dragging;
    int hover;
    int hidden;
    SDL_Texture* track_texture; /* optional */
    SDL_Texture* knob_texture;  /* optional */
    void (*on_change)(SDL_Context* ctx, int new_value);
} Crossfader;

/* Initialize crossfader subsystem */
void crossfaders_init(void);

/* Create a crossfader and register it. Returns pointer or NULL on error. */
Crossfader* crossfader_create(SDL_Renderer* renderer, int id, int x, int y, int w, int h, int min, int max, int value);

/* Render all registered crossfaders */
void crossfaders_render(SDL_Renderer* renderer);

/* Handle events for all crossfaders */
void crossfaders_handle_event(SDL_Context* ctx, SDL_Event* event);

/* Get crossfader by id */
Crossfader* crossfader_get(int id);

/* Set / get value by id */
int crossfader_get_value(int id);
int crossfader_set_value(int id, int value);

/* Set callback for a crossfader */
void crossfader_set_on_change(Crossfader* cf, void (*cb)(SDL_Context*, int));

/* Free all crossfaders */
void crossfaders_free(void);

#endif /* CROSSFADER_H */
