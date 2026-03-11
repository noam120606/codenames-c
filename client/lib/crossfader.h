#ifndef CROSSFADER_H
#define CROSSFADER_H

#include "../lib/sdl.h"

#define MAX_CROSSFADERS 32

/**
 * Configuration pour créer un `Crossfader`.
 * Tous les champs ont des valeurs par défaut définies via `crossfader_config_init`.
 * La structure peut être modifiée à tout moment et passée à `crossfader_create`.
 */
typedef struct CrossfaderConfig {
    /* --- champs configurables par l'utilisateur --- */
    int x;
    int y;
    int w;
    int h;
    int min;
    int max;
    int value;
    int hidden;
    void (*on_change)(SDL_Context* ctx, int new_value);

    /* --- champs d'état (runtime) --- */
    SDL_Rect rect;             /* full track rect */
    int dragging;
    int hover;
    SDL_Texture* track_texture; /* optional */
    SDL_Texture* knob_texture;  /* optional */
} CrossfaderConfig;

/**
 * Structure de crossfader.
 */
typedef struct Crossfader {
    int id;
    CrossfaderConfig* cfg;
} Crossfader;

/** Initialise une `CrossfaderConfig` avec des valeurs par défaut. */
CrossfaderConfig* crossfader_config_init(void);

/* Initialize crossfader subsystem */
void crossfaders_init(void);

/**
 * Crée un crossfader à partir d'une configuration.
 * Si `cfg` est NULL, les valeurs par défaut sont utilisées.
 */
Crossfader* crossfader_create(SDL_Renderer* renderer, int id, const CrossfaderConfig* cfg);

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
