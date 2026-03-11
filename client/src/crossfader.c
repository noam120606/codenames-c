/* Crossfader widget implementation */
#include "../lib/all.h"

static Crossfader* crossfaders[MAX_CROSSFADERS];
static int crossfader_count = 0;

/** Initialise une CrossfaderConfig avec des valeurs par défaut. */
CrossfaderConfig* crossfader_config_init(void) {
    CrossfaderConfig* cfg = (CrossfaderConfig*)calloc(1, sizeof(CrossfaderConfig));
    if (!cfg) return NULL;

    cfg->x = 0;
    cfg->y = 0;
    cfg->w = 100;
    cfg->h = 20;
    cfg->min = 0;
    cfg->max = 100;
    cfg->value = 50;
    cfg->hidden = 0;
    cfg->on_change = NULL;

    cfg->rect = (SDL_Rect){0, 0, 0, 0};
    cfg->dragging = 0;
    cfg->hover = 0;
    cfg->track_texture = NULL;
    cfg->knob_texture = NULL;

    return cfg;
}

void crossfaders_init(void) {
    for (int i = 0; i < MAX_CROSSFADERS; ++i) crossfaders[i] = NULL;
    crossfader_count = 0;
}

Crossfader* crossfader_create(SDL_Renderer* renderer, int id, const CrossfaderConfig* cfg_in) {
    (void)renderer;
    if (crossfader_count >= MAX_CROSSFADERS) return NULL;

    Crossfader* cf = (Crossfader*)malloc(sizeof(Crossfader));
    if (!cf) return NULL;
    cf->id = id;

    /* Crossfader owns its config. We copy cfg_in (or defaults) into a freshly allocated struct. */
    cf->cfg = crossfader_config_init();
    if (!cf->cfg) {
        free(cf);
        return NULL;
    }
    if (cfg_in) {
        /* Shallow copy first, then fix runtime fields below. */
        *cf->cfg = *cfg_in;
    }

    if (cf->cfg->min >= cf->cfg->max) {
        free(cf->cfg);
        free(cf);
        return NULL;
    }

    /* runtime-managed fields: always start from a clean state */
    cf->cfg->track_texture = NULL;
    cf->cfg->knob_texture = NULL;
    cf->cfg->dragging = 0;
    cf->cfg->hover = 0;

    /* clamp value */
    if (cf->cfg->value < cf->cfg->min) cf->cfg->value = cf->cfg->min;
    if (cf->cfg->value > cf->cfg->max) cf->cfg->value = cf->cfg->max;

    /* keep rect in sync */
    cf->cfg->rect = (SDL_Rect){cf->cfg->x, cf->cfg->y, cf->cfg->w, cf->cfg->h};

    crossfaders[crossfader_count++] = cf;
    return cf;
}

static Crossfader* find_by_id(int id) {
    for (int i = 0; i < crossfader_count; ++i) {
        if (crossfaders[i] && crossfaders[i]->id == id) return crossfaders[i];
    }
    return NULL;
}

Crossfader* crossfader_get(int id) {
    return find_by_id(id);
}

int crossfader_get_value(int id) {
    Crossfader* cf = find_by_id(id);
    if (!cf) return 0;
    return cf->cfg->value;
}

int crossfader_set_value(int id, int value) {
    Crossfader* cf = find_by_id(id);
    if (!cf) return EXIT_FAILURE;
    if (value < cf->cfg->min) value = cf->cfg->min;
    if (value > cf->cfg->max) value = cf->cfg->max;
    if (cf->cfg->value != value) {
        cf->cfg->value = value;
        if (cf->cfg->on_change) cf->cfg->on_change(NULL, cf->cfg->value);
    }
    return EXIT_SUCCESS;
}

void crossfader_set_on_change(Crossfader* cf, void (*cb)(SDL_Context*, int)) {
    if (!cf) return;
    cf->cfg->on_change = cb;
}

static int point_in_rect(int x, int y, SDL_Rect* r) {
    return x >= r->x && x <= (r->x + r->w) && y >= r->y && y <= (r->y + r->h);
}

/* Compute pixel X of knob center based on value */
static int knob_x_for_value(Crossfader* cf, int knob_w) {
    if (!cf) return 0;
    int range = cf->cfg->max - cf->cfg->min;
    if (range <= 0) return cf->cfg->rect.x + knob_w/2;
    double t = (double)(cf->cfg->value - cf->cfg->min) / (double)range;
    int span = cf->cfg->rect.w - knob_w;
    if (span < 0) span = 0;
    return cf->cfg->rect.x + (int)(t * span) + knob_w/2;
}

/* Update value from pixel x coordinate */
static void update_value_from_mouse(Crossfader* cf, int mouse_x) {
    int knob_w = cf->cfg->rect.h; /* use height as knob size */
    int left = cf->cfg->rect.x + knob_w/2;
    int right = cf->cfg->rect.x + cf->cfg->rect.w - knob_w/2;
    if (mouse_x < left) mouse_x = left;
    if (mouse_x > right) mouse_x = right;
    double t = (double)(mouse_x - left) / (double)(right - left);
    int newv = cf->cfg->min + (int)round(t * (cf->cfg->max - cf->cfg->min));
    if (newv < cf->cfg->min) newv = cf->cfg->min;
    if (newv > cf->cfg->max) newv = cf->cfg->max;
    if (newv != cf->cfg->value) {
        cf->cfg->value = newv;
        if (cf->cfg->on_change) cf->cfg->on_change(NULL, cf->cfg->value);
    }
}

void crossfaders_handle_event(SDL_Context* ctx, SDL_Event* event) {
    if (!event) return;
    if (event->type == SDL_MOUSEMOTION) {
        int mx = event->motion.x;
        int my = event->motion.y;
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf || cf->cfg->hidden) continue;
            cf->cfg->hover = point_in_rect(mx, my, &cf->cfg->rect);
            if (cf->cfg->dragging) update_value_from_mouse(cf, mx);
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf || cf->cfg->hidden) continue;
            if (point_in_rect(mx, my, &cf->cfg->rect)) {
                cf->cfg->dragging = 1;
                update_value_from_mouse(cf, mx);
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf) continue;
            cf->cfg->dragging = 0;
        }
    }
}

void crossfaders_render(SDL_Renderer* renderer) {
    if (!renderer) return;
    for (int i = 0; i < crossfader_count; ++i) {
        Crossfader* cf = crossfaders[i];
        if (!cf || cf->cfg->hidden) continue;

        /* Draw track */
        SDL_Rect track = cf->cfg->rect;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 200);
        SDL_RenderFillRect(renderer, &track);

        /* Draw progress filled portion */
        int knob_w = cf->cfg->rect.h; /* square knob */
        int left = cf->cfg->rect.x + knob_w/2;
        int right = cf->cfg->rect.x + cf->cfg->rect.w - knob_w/2;
        int kx = knob_x_for_value(cf, knob_w);
        SDL_Rect filled = { left, cf->cfg->rect.y, kx - left, cf->cfg->rect.h };
        if (filled.w < 0) filled.w = 0;
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderFillRect(renderer, &filled);

        /* Draw knob as rounded-ish rect */
        SDL_Rect knob = { kx - knob_w/2, cf->cfg->rect.y, knob_w, cf->cfg->rect.h };
        if (cf->cfg->hover || cf->cfg->dragging) {
            SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        }
        SDL_RenderFillRect(renderer, &knob);

        /* draw outline */
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderDrawRect(renderer, &track);
        SDL_RenderDrawRect(renderer, &knob);
    }
}

void crossfaders_free(void) {
    for (int i = 0; i < crossfader_count; ++i) {
        if (crossfaders[i]) {
            if (crossfaders[i]->cfg) {
                if (crossfaders[i]->cfg->track_texture) free_image(crossfaders[i]->cfg->track_texture);
                if (crossfaders[i]->cfg->knob_texture) free_image(crossfaders[i]->cfg->knob_texture);
                free(crossfaders[i]->cfg);
            }
            free(crossfaders[i]);
            crossfaders[i] = NULL;
        }
    }
    crossfader_count = 0;
}
