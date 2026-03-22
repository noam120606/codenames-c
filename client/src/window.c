#include "../lib/all.h"

static int clamp_non_negative(int value) {
	return value < 0 ? 0 : value;
}

static int clamp_positive(int value, int fallback) {
	if (value <= 0) return fallback;
	return value;
}

static int point_in_rect(int x, int y, const SDL_Rect* r) {
	if (!r) return 0;
	return x >= r->x && x < (r->x + r->w) && y >= r->y && y < (r->y + r->h);
}

static int point_in_titlebar(const Window* win, int x, int y) {
	if (!win || !win->cfg) return 0;
	const WindowConfig* cfg = win->cfg;
	SDL_Rect titlebar = {
		.x = cfg->rect.x,
		.y = cfg->rect.y,
		.w = cfg->rect.w,
		.h = cfg->titlebar_h
	};
	return point_in_rect(x, y, &titlebar);
}

WindowConfig* window_config_init(void) {
	WindowConfig* cfg = (WindowConfig*)calloc(1, sizeof(WindowConfig));
	if (!cfg) return NULL;

	cfg->x = 0;
	cfg->y = 0;
	cfg->w = 420;
	cfg->h = 320;
	cfg->movable = 1;
	cfg->hidden = 0;
	cfg->bg_color = (SDL_Color){28, 31, 38, 240};
	cfg->border_color = (SDL_Color){225, 229, 236, 255};
	cfg->titlebar_color = (SDL_Color){58, 76, 94, 255};
	cfg->border_thickness = 2;
	cfg->titlebar_h = 36;
	cfg->dragging = 0;
	cfg->drag_offset_x = 0;
	cfg->drag_offset_y = 0;

	cfg->rect.x = (WIN_WIDTH / 2) + cfg->x - (cfg->w / 2);
	cfg->rect.y = (WIN_HEIGHT / 2) - cfg->y - (cfg->h / 2);
	cfg->rect.w = cfg->w;
	cfg->rect.h = cfg->h;

	return cfg;
}

void window_update_rect(Window* win) {
	if (!win || !win->cfg) return;

	WindowConfig* cfg = win->cfg;
	cfg->w = clamp_positive(cfg->w, 1);
	cfg->h = clamp_positive(cfg->h, 1);
	cfg->titlebar_h = clamp_non_negative(cfg->titlebar_h);

	cfg->rect.w = cfg->w;
	cfg->rect.h = cfg->h;
	cfg->rect.x = (WIN_WIDTH / 2) + cfg->x - (cfg->w / 2);
	cfg->rect.y = (WIN_HEIGHT / 2) - cfg->y - (cfg->h / 2);
}

Window* window_create(int id, const WindowConfig* cfg_in) {
	Window* win = (Window*)calloc(1, sizeof(Window));
	if (!win) return NULL;

	win->id = id;
	win->cfg = window_config_init();
	if (!win->cfg) {
		free(win);
		return NULL;
	}

	if (cfg_in) {
		*(win->cfg) = *cfg_in;
	}

	window_update_rect(win);
	return win;
}

void window_destroy(Window* win) {
	if (!win) return;
	free(win->cfg);
	free(win);
}

int window_contains_point(const Window* win, int logical_x, int logical_y) {
	if (!win || !win->cfg || win->cfg->hidden) return 0;
	return point_in_rect(logical_x, logical_y, &win->cfg->rect);
}

void window_handle_event(AppContext* ctx, Window* win, SDL_Event* event) {
	if (!ctx || !win || !win->cfg || !event) return;

	WindowConfig* cfg = win->cfg;
	if (cfg->hidden) return;

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
		int mx = event->button.x;
		int my = event->button.y;

		if (cfg->movable && point_in_titlebar(win, mx, my)) {
			cfg->dragging = 1;
			cfg->drag_offset_x = mx - cfg->rect.x;
			cfg->drag_offset_y = my - cfg->rect.y;
		}
	} else if (event->type == SDL_MOUSEMOTION) {
		if (!cfg->dragging) return;

		int mx = event->motion.x;
		int my = event->motion.y;

		int new_rect_x = mx - cfg->drag_offset_x;
		int new_rect_y = my - cfg->drag_offset_y;

		cfg->x = new_rect_x + (cfg->w / 2) - (WIN_WIDTH / 2);
		cfg->y = (WIN_HEIGHT / 2) - (new_rect_y + (cfg->h / 2));
		window_update_rect(win);
	} else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
		cfg->dragging = 0;
	}
}

void window_render(SDL_Renderer* renderer, const Window* win) {
	if (!renderer || !win || !win->cfg || win->cfg->hidden) return;

	const WindowConfig* cfg = win->cfg;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_SetRenderDrawColor(renderer, cfg->bg_color.r, cfg->bg_color.g, cfg->bg_color.b, cfg->bg_color.a);
	SDL_RenderFillRect(renderer, &cfg->rect);

	if (cfg->titlebar_h > 0) {
		SDL_Rect titlebar = {
			.x = cfg->rect.x,
			.y = cfg->rect.y,
			.w = cfg->rect.w,
			.h = cfg->titlebar_h
		};
		SDL_SetRenderDrawColor(renderer, cfg->titlebar_color.r, cfg->titlebar_color.g, cfg->titlebar_color.b, cfg->titlebar_color.a);
		SDL_RenderFillRect(renderer, &titlebar);
	}

	int border = clamp_non_negative(cfg->border_thickness);
	SDL_SetRenderDrawColor(renderer, cfg->border_color.r, cfg->border_color.g, cfg->border_color.b, cfg->border_color.a);
	for (int i = 0; i < border; i++) {
		SDL_Rect r = {
			.x = cfg->rect.x + i,
			.y = cfg->rect.y + i,
			.w = cfg->rect.w - (2 * i),
			.h = cfg->rect.h - (2 * i)
		};
		if (r.w <= 0 || r.h <= 0) break;
		SDL_RenderDrawRect(renderer, &r);
	}
}

int window_edit_cfg(Window* win, WindowCfgKey key, intptr_t value) {
	if (!win || !win->cfg) return EXIT_FAILURE;

	WindowConfig* cfg = win->cfg;
	int need_rect_refresh = 0;

	switch (key) {
		case WIN_CFG_X: cfg->x = (int)value; need_rect_refresh = 1; break;
		case WIN_CFG_Y: cfg->y = (int)value; need_rect_refresh = 1; break;
		case WIN_CFG_W: cfg->w = (int)value; need_rect_refresh = 1; break;
		case WIN_CFG_H: cfg->h = (int)value; need_rect_refresh = 1; break;
		case WIN_CFG_MOVABLE: cfg->movable = ((int)value != 0); break;
		case WIN_CFG_HIDDEN: cfg->hidden = ((int)value != 0); break;
		case WIN_CFG_BG_COLOR: cfg->bg_color = *((SDL_Color*)value); break;
		case WIN_CFG_BORDER_COLOR: cfg->border_color = *((SDL_Color*)value); break;
		case WIN_CFG_TITLEBAR_COLOR: cfg->titlebar_color = *((SDL_Color*)value); break;
		case WIN_CFG_BORDER_THICKNESS: cfg->border_thickness = (int)value; break;
		case WIN_CFG_TITLEBAR_H: cfg->titlebar_h = (int)value; break;
		case WIN_CFG_RECT: cfg->rect = *((SDL_Rect*)value); break;
		case WIN_CFG_DRAGGING: cfg->dragging = ((int)value != 0); break;
		case WIN_CFG_DRAG_OFFSET_X: cfg->drag_offset_x = (int)value; break;
		case WIN_CFG_DRAG_OFFSET_Y: cfg->drag_offset_y = (int)value; break;
		default:
			return EXIT_FAILURE;
	}

	if (need_rect_refresh) {
		window_update_rect(win);
	}

	return EXIT_SUCCESS;
}
