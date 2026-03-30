/**
 * @file crossfader.h
 * @brief Gestion des curseurs de réglage (volume, paramètres).
 */

#ifndef CROSSFADER_H
#define CROSSFADER_H

#include <stdint.h>
#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

typedef struct AppContext AppContext;

#define MAX_CROSSFADERS 32

/**
 * Configuration pour créer un `Crossfader`.
 * Tous les champs ont des valeurs par défaut définies via `crossfader_config_init`.
 * La structure peut être modifiée à tout moment et passée à `crossfader_create`.
 * @
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
    void (*on_change)(AppContext* ctx, int new_value);

    /* --- champs d'état (runtime) --- */
    SDL_Rect rect;             /* full track rect */
    int dragging;
    int save_pending;          /* flag: value changed while dragging, write on release */
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

/**
 * Clés de configuration modifiables via `edit_crfd_cfg`.
 */
typedef enum CrfdCfgKey {
    CRFD_CFG_X = 100,
    CRFD_CFG_Y,
    CRFD_CFG_W,
    CRFD_CFG_H,
    CRFD_CFG_MIN,
    CRFD_CFG_MAX,
    CRFD_CFG_VALUE,
    CRFD_CFG_HIDDEN,
    CRFD_CFG_SAVE_PLAYER_DATA,
    CRFD_CFG_COLOR_0_PCT,
    CRFD_CFG_COLOR_100_PCT,
    CRFD_CFG_KNOB_COLOR,
    CRFD_CFG_ON_CHANGE,
    CRFD_CFG_RECT,
    CRFD_CFG_DRAGGING,
    CRFD_CFG_SAVE_PENDING,
    CRFD_CFG_HOVER,
    CRFD_CFG_TRACK_TEXTURE,
    CRFD_CFG_KNOB_TEXTURE,
} CrfdCfgKey;

/**
 * Initialise une `CrossfaderConfig` avec des valeurs par défaut.
 * @return Pointeur vers la configuration allouée, ou NULL en cas d'échec.
 */
CrossfaderConfig* crossfader_config_init(void);

/**
 * Initialise le sous-système des crossfaders.
 */
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

/**
 * Affiche tous les crossfaders enregistrés.
 * @param renderer Renderer SDL.
 */
void crossfaders_render(SDL_Renderer* renderer);

/**
 * Gère les événements pour tous les crossfaders (mouvement de la souris, clics).
 * Doit être appelé depuis la boucle principale d'événements.
 * @param ctx Contexte SDL, passé aux callbacks.
 * @param event Événement SDL à traiter.
 */
void crossfaders_handle_event(AppContext* ctx, SDL_Event* event);

/**
 * Récupère un crossfader par son identifiant.
 * @param id Identifiant du crossfader.
 * @return Pointeur vers le crossfader avec l'id donné, ou NULL si non trouvé.
 */
Crossfader* crossfader_get(int id);

/**
 * Récupère la valeur d'un crossfader par son identifiant.
 * @param id Identifiant du crossfader.
 * @return Valeur du crossfader avec l'id donné, ou 0 si non trouvé.
 */
int crossfader_get_value(int id);

/** 
 * Définit la valeur d'un crossfader par son identifiant.
 * La valeur sera limitée à [min, max] et déclenchera on_change si différente de la valeur actuelle.
 * @param id Identifiant du crossfader.
 * @param value Nouvelle valeur à définir.
 * @return EXIT_SUCCESS si la valeur a été définie avec succès, EXIT_FAILURE si le crossfader n'a pas été trouvé.
 */
int crossfader_set_value(int id, int value);

/**
 * Définit le callback d'un crossfader.
 * @param cf Pointeur vers le crossfader.
 * @param cb Fonction callback à appeler lors du changement de valeur.
 */
void crossfader_set_on_change(Crossfader* cf, void (*cb)(AppContext*, int));

/**
 * Libère tous les crossfaders.
 */
void crossfaders_free(void);

/**
 * Modifie une valeur de configuration d'un crossfader identifié par son id.
 * @param id    L'ID du crossfader à modifier.
 * @param key   Champ ciblé dans la configuration.
 * @param value Valeur à appliquer (entier direct pour champs int/bool, pointeur casté en `intptr_t` pour les champs pointeurs, et adresse d'une structure pour `CRFD_CFG_COLOR_0_PCT` / `CRFD_CFG_COLOR_100_PCT` / `CRFD_CFG_KNOB_COLOR` / `CRFD_CFG_RECT`).
 * @return EXIT_SUCCESS si le crossfader existe, sinon EXIT_FAILURE.
 */
int edit_crfd_cfg(int id, CrfdCfgKey key, intptr_t value);

#endif /* CROSSFADER_H */
