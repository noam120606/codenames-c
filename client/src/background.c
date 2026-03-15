#include "../lib/all.h"

/* Textures */
static SDL_Texture* lens;
static SDL_Texture* hat;
static SDL_Texture* spyglasses;

/* Constantes de la grille (partagées init/display/event) */
#define BG_SYMBOL_W  64
#define BG_SYMBOL_H  64
#define BG_SPACING   32
#define BG_TILE_W    (BG_SYMBOL_W + BG_SPACING)
#define BG_TILE_H    (BG_SYMBOL_H + BG_SPACING)
#define BG_EXTRA     15
#define BG_SPEED     0.55f
#define BG_SIZE      0.12f
#define BG_ANIM_FRAMES 14
#define BG_ANIM_POP    0.18f

/* Table des états surchargés par clic.  */
#define MAX_OVERRIDES 512
typedef struct { int wi; int wj; int symbol; } TileOverride;
static TileOverride overrides[MAX_OVERRIDES];
static int override_count = 0;

typedef struct {
    int wi;
    int wj;
    int from_symbol;
    int to_symbol;
    long start_clock;
    int active;
} TileAnimation;

static TileAnimation animations[MAX_OVERRIDES];

static int find_override(int wi, int wj) {
    for (int k = 0; k < override_count; k++)
        if (overrides[k].wi == wi && overrides[k].wj == wj) return k;
    return -1;
}

static void set_override(int wi, int wj, int symbol) {
    int idx = find_override(wi, wj);
    if (idx >= 0) {
        overrides[idx].symbol = symbol;
    } else if (override_count < MAX_OVERRIDES) {
        overrides[override_count].wi     = wi;
        overrides[override_count].wj     = wj;
        overrides[override_count].symbol = symbol;
        override_count++;
    }
}

static int find_animation(int wi, int wj) {
    for (int k = 0; k < MAX_OVERRIDES; k++)
        if (animations[k].active && animations[k].wi == wi && animations[k].wj == wj) return k;
    return -1;
}

static void start_animation(int wi, int wj, int from_symbol, int to_symbol, long start_clock) {
    int idx = find_animation(wi, wj);

    if (idx < 0) {
        for (int k = 0; k < MAX_OVERRIDES; k++) {
            if (!animations[k].active) {
                idx = k;
                break;
            }
        }
    }

    if (idx < 0) return;

    animations[idx].wi = wi;
    animations[idx].wj = wj;
    animations[idx].from_symbol = from_symbol;
    animations[idx].to_symbol = to_symbol;
    animations[idx].start_clock = start_clock;
    animations[idx].active = 1;
}

static SDL_Texture* symbol_texture(int symbol) {
    switch (symbol) {
        case 1: return hat ? hat : (lens ? lens : spyglasses);
        case 2: return spyglasses ? spyglasses : (lens ? lens : hat);
        default: return lens ? lens : (hat ? hat : spyglasses);
    }
}

/* Symbole par défaut : principalement loupe (80 %), hat/spyglasses rares (10 % chacun) */
static int default_symbol(int wi, int wj) {
    const unsigned int h = ((unsigned int)wi * 73856093u) ^ ((unsigned int)wj * 19349663u);
    unsigned int r = h % 10u;
    if (r < 8u) return 0; /* lens */
    if (r == 8u) return 1; /* hat */
    return 2;              /* spyglasses */
}

/*
 * Phase monde stable.
 * L'offset visuel wrap tous les TILE*2 unités de scaled_time.
 * La valeur retournée est toujours un multiple de 2 (période quinconce).
 */
static int world_phase(int scaled_time, int tile_size) {
    return 2 * (scaled_time / (tile_size * 2));
}

static void window_to_logical(SDL_Context* context, int wx, int wy, int* lx, int* ly) {
    if (!lx || !ly) return;

    if (!context || !context->renderer) {
        *lx = wx;
        *ly = wy;
        return;
    }

    float logical_x = (float)wx;
    float logical_y = (float)wy;
    SDL_RenderWindowToLogical(context->renderer, wx, wy, &logical_x, &logical_y);
    *lx = (int)lroundf(logical_x);
    *ly = (int)lroundf(logical_y);
}


int init_background(SDL_Context* context) {
    int fails = 0;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    lens = load_image(context->renderer, "assets/img/background/lens.png");
    if (!lens)       { printf("Failed to load lens image\n");       fails++; }

    hat  = load_image(context->renderer, "assets/img/background/hat.png");
    if (!hat)        { printf("Failed to load hat image\n");        fails++; }

    spyglasses = load_image(context->renderer, "assets/img/background/spyglasses.png");
    if (!spyglasses) { printf("Failed to load spyglasses image\n"); fails++; }

    if (lens)       SDL_SetTextureScaleMode(lens,       SDL_ScaleModeNearest);
    if (hat)        SDL_SetTextureScaleMode(hat,        SDL_ScaleModeNearest);
    if (spyglasses) SDL_SetTextureScaleMode(spyglasses, SDL_ScaleModeNearest);

    return fails;
}

void display_background(SDL_Context* context) {
    const int   INTERACT_RADIUS = 150;
    const float MAX_SCALE       = 1.4f;

    const int cols = (WIN_WIDTH  / BG_TILE_W) + BG_EXTRA;
    const int rows = (WIN_HEIGHT / BG_TILE_H) + BG_EXTRA;

    const int scaled_time = (int)(context->clock * BG_SPEED);
    const int offset_x = scaled_time % (BG_TILE_W * 2);
    const int offset_y = scaled_time % (BG_TILE_H * 2);
    const int phase_x  = world_phase(scaled_time, BG_TILE_W);
    const int phase_y  = world_phase(scaled_time, BG_TILE_H);

    int raw_mx, raw_my;
    SDL_GetMouseState(&raw_mx, &raw_my);
    int logical_mx = raw_mx;
    int logical_my = raw_my;
    window_to_logical(context, raw_mx, raw_my, &logical_mx, &logical_my);
    const int mouse_x =  logical_mx - WIN_WIDTH  / 2;
    const int mouse_y = (WIN_HEIGHT / 2) - logical_my;

    for (int i = -BG_EXTRA; i < cols; i++) {
        const int row_offset = (i % 2) ? BG_TILE_H / 2 : 0;
        const int wi = i + phase_x;

        for (int j = -BG_EXTRA; j < rows; j++) {
            const int x  = (i * BG_TILE_W) - offset_x;
            const int y  = (j * BG_TILE_H) - offset_y + row_offset;
            const int wj = j + phase_y;

            /* Symbole : override éventuel, sinon distribution par défaut */
            int idx = find_override(wi, wj);
            int sym = (idx >= 0) ? overrides[idx].symbol : default_symbol(wi, wj);
            SDL_Texture* symbol = symbol_texture(sym);
            if (!symbol) continue;

            /* Effet grossissement souris */
            const int   dx   = x + (int)(BG_SYMBOL_W * BG_SIZE / 2) - mouse_x;
            const int   dy   = y + (int)(BG_SYMBOL_H * BG_SIZE / 2) - mouse_y;
            const int   dist = (int)sqrt((double)(dx*dx + dy*dy));
            float scale = 1.0f;
            if (dist < INTERACT_RADIUS)
                scale = 1.0f + (1.0f - (float)dist / INTERACT_RADIUS) * (MAX_SCALE - 1.0f);

            int anim_idx = find_animation(wi, wj);
            if (anim_idx >= 0) {
                long elapsed = context->clock - animations[anim_idx].start_clock;
                if (elapsed >= BG_ANIM_FRAMES) {
                    animations[anim_idx].active = 0;
                } else {
                    float t = (float)elapsed / (float)BG_ANIM_FRAMES;
                    float pulse = 1.0f + ((t <= 0.5f ? t : (1.0f - t)) * 2.0f) * BG_ANIM_POP;
                    Uint8 old_opacity = (Uint8)(255.0f * (1.0f - t));
                    Uint8 new_opacity = (Uint8)(255.0f * t);
                    SDL_Texture* from_texture = symbol_texture(animations[anim_idx].from_symbol);
                    SDL_Texture* to_texture = symbol_texture(animations[anim_idx].to_symbol);

                    if (from_texture) {
                        display_image(context->renderer, from_texture, x, y, BG_SIZE * scale * pulse, 0,
                                      row_offset == 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 1, old_opacity);
                    }
                    if (to_texture) {
                        display_image(context->renderer, to_texture, x, y, BG_SIZE * scale * pulse, 0,
                                      row_offset == 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 1, new_opacity);
                    }
                    continue;
                }
            }

            display_image(context->renderer, symbol, x, y, BG_SIZE * scale, 0,
                          row_offset == 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL, 1, 255);
        }
    }
}

/* Event (clic gauche → cycle symbole du tile cliqué) */
void background_handle_event(SDL_Context* context, SDL_Event* e) {
    if (e->type != SDL_MOUSEBUTTONDOWN || e->button.button != SDL_BUTTON_LEFT) return;

    const int scaled_time = (int)(context->clock * BG_SPEED);
    const int offset_x = scaled_time % (BG_TILE_W * 2);
    const int offset_y = scaled_time % (BG_TILE_H * 2);
    const int phase_x  = world_phase(scaled_time, BG_TILE_W);
    const int phase_y  = world_phase(scaled_time, BG_TILE_H);

    int logical_click_x = e->button.x;
    int logical_click_y = e->button.y;
    window_to_logical(context, e->button.x, e->button.y, &logical_click_x, &logical_click_y);

    /* Coordonnées clic dans le repère centré de display_image */
    const int click_x =  logical_click_x - WIN_WIDTH  / 2;
    const int click_y = (WIN_HEIGHT / 2) - logical_click_y;

    const int cols = (WIN_WIDTH  / BG_TILE_W) + BG_EXTRA;
    const int rows = (WIN_HEIGHT / BG_TILE_H) + BG_EXTRA;

    int best_dist2 = (BG_TILE_W / 2) * (BG_TILE_W / 2);
    int best_wi = 0, best_wj = 0;
    int found = 0;

    for (int i = -BG_EXTRA; i < cols; i++) {
        const int row_offset = (i % 2) ? BG_TILE_H / 2 : 0;
        const int wi = i + phase_x;

        for (int j = -BG_EXTRA; j < rows; j++) {
            const int x  = (i * BG_TILE_W) - offset_x;
            const int y  = (j * BG_TILE_H) - offset_y + row_offset;
            const int cx = x + (int)(BG_SYMBOL_W * BG_SIZE / 2);
            const int cy = y + (int)(BG_SYMBOL_H * BG_SIZE / 2);
            const int dx = cx - click_x;
            const int dy = cy - click_y;
            const int dist2 = dx*dx + dy*dy;
            if (dist2 < best_dist2) {
                best_dist2 = dist2;
                best_wi = wi;
                best_wj = j + phase_y;
                found = 1;
            }
        }
    }

    if (!found) return;

    /* Cycle : lens(0) hat(1) spyglasses(2) lens(0) */
    int idx     = find_override(best_wi, best_wj);
    int current = (idx >= 0) ? overrides[idx].symbol : default_symbol(best_wi, best_wj);
    int next = (current + 1) % 3;

    set_override(best_wi, best_wj, next);
    start_animation(best_wi, best_wj, current, next, context->clock);
}


int destroy_background() {
    if (lens)       { free_image(lens);       lens       = NULL; }
    if (hat)        { free_image(hat);        hat        = NULL; }
    if (spyglasses) { free_image(spyglasses); spyglasses = NULL; }
    override_count = 0;
    memset(animations, 0, sizeof(animations));
    return EXIT_SUCCESS;
}