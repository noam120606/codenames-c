/* Crossfader widget implementation */
#include "../lib/all.h"

static Crossfader* crossfaders[MAX_CROSSFADERS];
static int crossfader_count = 0;

void crossfaders_init(void) {
    for (int i = 0; i < MAX_CROSSFADERS; ++i) crossfaders[i] = NULL;
    crossfader_count = 0;
}

Crossfader* crossfader_create(SDL_Renderer* renderer, int id, int x, int y, int w, int h, int min, int max, int value) {
    if (crossfader_count >= MAX_CROSSFADERS) return NULL;
    if (min >= max) return NULL;

    Crossfader* cf = (Crossfader*)malloc(sizeof(Crossfader));
    if (!cf) return NULL;
    cf->id = id;
    cf->rect.x = x;
    cf->rect.y = y;
    cf->rect.w = w;
    cf->rect.h = h;
    cf->min = min;
    cf->max = max;
    if (value < min) value = min;
    if (value > max) value = max;
    cf->value = value;
    cf->dragging = 0;
    cf->hover = 0;
    cf->hidden = 0;
    cf->track_texture = NULL;
    cf->knob_texture = NULL;
    cf->on_change = NULL;

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
    return cf->value;
}

int crossfader_set_value(int id, int value) {
    Crossfader* cf = find_by_id(id);
    if (!cf) return EXIT_FAILURE;
    if (value < cf->min) value = cf->min;
    if (value > cf->max) value = cf->max;
    if (cf->value != value) {
        cf->value = value;
        if (cf->on_change) cf->on_change(NULL, cf->value);
    }
    return EXIT_SUCCESS;
}

void crossfader_set_on_change(Crossfader* cf, void (*cb)(SDL_Context*, int)) {
    if (!cf) return;
    cf->on_change = cb;
}

static int point_in_rect(int x, int y, SDL_Rect* r) {
    return x >= r->x && x <= (r->x + r->w) && y >= r->y && y <= (r->y + r->h);
}

/* Compute pixel X of knob center based on value */
static int knob_x_for_value(Crossfader* cf, int knob_w) {
    if (!cf) return 0;
    int range = cf->max - cf->min;
    if (range <= 0) return cf->rect.x + knob_w/2;
    double t = (double)(cf->value - cf->min) / (double)range;
    int span = cf->rect.w - knob_w;
    if (span < 0) span = 0;
    return cf->rect.x + (int)(t * span) + knob_w/2;
}

/* Update value from pixel x coordinate */
static void update_value_from_mouse(Crossfader* cf, int mouse_x) {
    int knob_w = cf->rect.h; /* use height as knob size */
    int left = cf->rect.x + knob_w/2;
    int right = cf->rect.x + cf->rect.w - knob_w/2;
    if (mouse_x < left) mouse_x = left;
    if (mouse_x > right) mouse_x = right;
    double t = (double)(mouse_x - left) / (double)(right - left);
    int newv = cf->min + (int)round(t * (cf->max - cf->min));
    if (newv < cf->min) newv = cf->min;
    if (newv > cf->max) newv = cf->max;
    if (newv != cf->value) {
        cf->value = newv;
        if (cf->on_change) cf->on_change(NULL, cf->value);
    }
}

void crossfaders_handle_event(SDL_Context* ctx, SDL_Event* event) {
    if (!event) return;
    if (event->type == SDL_MOUSEMOTION) {
        int mx = event->motion.x;
        int my = event->motion.y;
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf || cf->hidden) continue;
            cf->hover = point_in_rect(mx, my, &cf->rect);
            if (cf->dragging) update_value_from_mouse(cf, mx);
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf || cf->hidden) continue;
            if (point_in_rect(mx, my, &cf->rect)) {
                cf->dragging = 1;
                update_value_from_mouse(cf, mx);
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        for (int i = 0; i < crossfader_count; ++i) {
            Crossfader* cf = crossfaders[i];
            if (!cf) continue;
            cf->dragging = 0;
        }
    }
}

void crossfaders_render(SDL_Renderer* renderer) {
    if (!renderer) return;
    for (int i = 0; i < crossfader_count; ++i) {
        Crossfader* cf = crossfaders[i];
        if (!cf || cf->hidden) continue;

        /* Draw track */
        SDL_Rect track = cf->rect;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 200);
        SDL_RenderFillRect(renderer, &track);

        /* Draw progress filled portion */
        int knob_w = cf->rect.h; /* square knob */
        int left = cf->rect.x + knob_w/2;
        int right = cf->rect.x + cf->rect.w - knob_w/2;
        int kx = knob_x_for_value(cf, knob_w);
        SDL_Rect filled = { left, cf->rect.y, kx - left, cf->rect.h };
        if (filled.w < 0) filled.w = 0;
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderFillRect(renderer, &filled);

        /* Draw knob as rounded-ish rect */
        SDL_Rect knob = { kx - knob_w/2, cf->rect.y, knob_w, cf->rect.h };
        if (cf->hover || cf->dragging) {
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
            if (crossfaders[i]->track_texture) free_image(crossfaders[i]->track_texture);
            if (crossfaders[i]->knob_texture) free_image(crossfaders[i]->knob_texture);
            free(crossfaders[i]);
            crossfaders[i] = NULL;
        }
    }
    crossfader_count = 0;
}
