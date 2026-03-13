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
    int save_player_data;     /* booléen: sauvegarde la valeur en temps réel */
    SDL_Color color_0_pct;    /* couleur de la track à 0% */
    SDL_Color color_100_pct;  /* couleur de la track à 100% */
    SDL_Color knob_color;     /* couleur du noeud */
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
 * Le crossfader créé doit être libéré avec `crossfaders_free` pour éviter les fuites de mémoire.
 * @param renderer Renderer SDL (actuellement inutilisé, mais peut être utilisé à l'avenir pour charger des textures).
 * @param id Identifiant du crossfader, utilisé pour le récupérer plus tard. Doit être unique parmi les crossfaders créés.
 * @param cfg_in Pointeur vers la configuration du crossfader. Si NULL, les valeurs par défaut sont utilisées. La configuration est copiée, donc le pointeur peut être libéré ou modifié après l'appel sans affecter le crossfader créé.
 * @return Pointeur vers le crossfader créé, ou NULL en cas d'erreur (par exemple, échec d'allocation mémoire ou si le nombre maximum de crossfaders est atteint).
 */
Crossfader* crossfader_create(SDL_Renderer* renderer, int id, const CrossfaderConfig* cfg);

/* Render all registered crossfaders */
void crossfaders_render(SDL_Renderer* renderer);

/** Handle events for all crossfaders (mouse motion, clicks). Must be called from the main event loop.
 * @param ctx SDL context, passed to callbacks.
 * @param event SDL event to handle.
 */
void crossfaders_handle_event(SDL_Context* ctx, SDL_Event* event);

/** Get crossfader by id.
 * @param id Crossfader id.
 * @return Pointer to the crossfader with the given id, or NULL if not found.
 */
Crossfader* crossfader_get(int id);

/** Get value of a crossfader by id.
 * @param id Crossfader id.
 * @return Value of the crossfader with the given id, or 0 if not found.
 */
int crossfader_get_value(int id);
/** 
 * Set value of a crossfader by id. Will be clamped to [min, max] and trigger on_change if different from current value.
 * @param id Crossfader id.
 * @param value New value to set.
 * @return EXIT_SUCCESS if the value was set successfully, EXIT_FAILURE if the crossfader with the given id was not found.
 */
int crossfader_set_value(int id, int value);

/** Set callback for a crossfader.
 * @param cf Crossfader pointer.
 * @param cb Callback function.
 */
void crossfader_set_on_change(Crossfader* cf, void (*cb)(SDL_Context*, int));

/** Free all crossfaders.
 */
void crossfaders_free(void);

#endif /* CROSSFADER_H */
