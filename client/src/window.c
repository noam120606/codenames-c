#include "../lib/all.h"

static Window* windows[MAX_WINDOWS];
static int window_count = 0;

static int clamp_non_negative(int value) {
	return value < 0 ? 0 : value;
}

static int clamp_positive(int value, int fallback) {
	if (value <= 0) return fallback;
	return value;
}

static int clamp_between(int value, int min_value, int max_value) {
	if (min_value > max_value) {
		int tmp = min_value;
		min_value = max_value;
		max_value = tmp;
	}

	if (value < min_value) return min_value;
	if (value > max_value) return max_value;
	return value;
}

static int point_in_rect(int x, int y, const SDL_Rect* r) {
	if (!r) return 0;
	return x >= r->x && x < (r->x + r->w) && y >= r->y && y < (r->y + r->h);
}

static void window_normalize_scroll_cfg(WindowConfig* cfg) {
	if (!cfg) return;

	if (cfg->scroll_min > cfg->scroll_max) {
		int tmp = cfg->scroll_min;
		cfg->scroll_min = cfg->scroll_max;
		cfg->scroll_max = tmp;
	}

	cfg->scroll_step = clamp_positive(cfg->scroll_step, 1);
	cfg->scroll_offset = clamp_between(cfg->scroll_offset, cfg->scroll_min, cfg->scroll_max);
}

static int window_scrollable_zone_rect_from_cfg(const WindowConfig* cfg, SDL_Rect* out_rect) {
	if (!cfg || !out_rect || !cfg->scrollable) return EXIT_FAILURE;

	int center_x = cfg->rect.x + (cfg->rect.w / 2);
	int center_y = cfg->rect.y + (cfg->rect.h / 2);

	/* x vers la droite, y vers le haut (repère local fenêtre) */
	int p1_x = center_x + cfg->scroll_x1;
	int p1_y = center_y - cfg->scroll_y1;
	int p2_x = center_x + cfg->scroll_x2;
	int p2_y = center_y - cfg->scroll_y2;

	int left = (p1_x < p2_x) ? p1_x : p2_x;
	int right = (p1_x > p2_x) ? p1_x : p2_x;
	int top = (p1_y < p2_y) ? p1_y : p2_y;
	int bottom = (p1_y > p2_y) ? p1_y : p2_y;

	out_rect->x = left;
	out_rect->y = top;
	out_rect->w = right - left;
	out_rect->h = bottom - top;

	if (out_rect->w <= 0 || out_rect->h <= 0) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static void window_to_logical_coords(const AppContext* ctx, int wx, int wy, int* lx, int* ly) {
	if (!lx || !ly) return;

	if (!ctx || !ctx->renderer) {
		*lx = wx;
		*ly = wy;
		return;
	}

	float logical_x = (float)wx;
	float logical_y = (float)wy;
	SDL_RenderWindowToLogical(ctx->renderer, wx, wy, &logical_x, &logical_y);
	*lx = (int)lroundf(logical_x);
	*ly = (int)lroundf(logical_y);
}

static int window_content_origin_screen(const Window* win, int* out_x, int* out_y) {
	if (!win || !win->cfg) return EXIT_FAILURE;
	if (out_x) *out_x = win->cfg->rect.x + (win->cfg->rect.w / 2);
	if (out_y) *out_y = win->cfg->rect.y + (win->cfg->rect.h / 2);
	return EXIT_SUCCESS;
}

static int window_set_title(WindowConfig* cfg, const char* title) {
	if (!cfg) return EXIT_FAILURE;

	char* new_title = NULL;
	if (title && title[0] != '\0') {
		new_title = strdup(title);
		if (!new_title) return EXIT_FAILURE;
	}

	free(cfg->title);
	cfg->title = new_title;
	return EXIT_SUCCESS;
}

static void window_render_title(SDL_Renderer* renderer, const WindowConfig* cfg) {
	if (!renderer || !cfg || !cfg->title || cfg->title[0] == '\0' || cfg->titlebar_h <= 0) return;

	int font_size = cfg->titlebar_h - 12;
	if (font_size < 12) font_size = 12;

	TTF_Font* font = TTF_OpenFont(FONT_LARABIE, font_size);
	if (!font) return;

	SDL_Surface* title_surface = TTF_RenderUTF8_Blended(font, cfg->title, COL_WHITE);
	if (!title_surface) {
		TTF_CloseFont(font);
		return;
	}

	SDL_Texture* title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
	if (!title_texture) {
		SDL_FreeSurface(title_surface);
		TTF_CloseFont(font);
		return;
	}

	SDL_Rect dst = {
		.x = cfg->rect.x + 10,
		.y = cfg->rect.y + (cfg->titlebar_h - title_surface->h) / 2,
		.w = title_surface->w,
		.h = title_surface->h
	};

	SDL_Rect clip = {
		.x = cfg->rect.x,
		.y = cfg->rect.y,
		.w = cfg->rect.w,
		.h = cfg->titlebar_h
	};

	SDL_RenderSetClipRect(renderer, &clip);
	SDL_RenderCopy(renderer, title_texture, NULL, &dst);
	SDL_RenderSetClipRect(renderer, NULL);

	SDL_DestroyTexture(title_texture);
	SDL_FreeSurface(title_surface);
	TTF_CloseFont(font);
}

WindowConfig* window_config_init(void) {
	WindowConfig* cfg = (WindowConfig*)calloc(1, sizeof(WindowConfig));
	if (!cfg) return NULL;

	cfg->x = 0;
	cfg->y = 0;
	cfg->w = 420;
	cfg->h = 320;
	cfg->movable = 0;
	cfg->hidden = 0;
	cfg->bg_color = (SDL_Color){20, 20, 20, 220}; // Noir semi-transparent par défaut
	cfg->border_color = (SDL_Color){230, 230, 230, 255}; // Blanc par défaut
	cfg->titlebar_color = (SDL_Color){40, 40, 100, 255}; // Bleu par défaut
	cfg->window_texture = NULL;
	cfg->titlebar_texture = NULL;
	cfg->title = NULL;
	cfg->border_thickness = 2;
	cfg->titlebar_h = 36;
	cfg->scrollable = 0;
	cfg->scroll_x1 = 0;
	cfg->scroll_y1 = 0;
	cfg->scroll_x2 = 0;
	cfg->scroll_y2 = 0;
	cfg->scroll_offset = 0;
	cfg->scroll_min = 0;
	cfg->scroll_max = 0;
	cfg->scroll_step = 1;
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

	window_normalize_scroll_cfg(cfg);
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
		const char* incoming_title = cfg_in->title;
		*(win->cfg) = *cfg_in;
		win->cfg->title = NULL;
		if (window_set_title(win->cfg, incoming_title) != EXIT_SUCCESS) {
			window_destroy(win);
			return NULL;
		}
	}

	window_update_rect(win);
	windows[window_count++] = win;
	return win;
}

Window* window_get_by_id(int id) {
	for (int i = 0; i < window_count; i++) {
		if (windows[i] && windows[i]->id == id) {
			return windows[i];
		}
	}
	return NULL;
}

void window_destroy(Window* win) {
	if (!win) return;
	if (win->cfg) {
		if (win->cfg->window_texture == win->cfg->titlebar_texture) {
			if (win->cfg->window_texture) free_image(win->cfg->window_texture);
		} else {
			if (win->cfg->window_texture) free_image(win->cfg->window_texture);
			if (win->cfg->titlebar_texture) free_image(win->cfg->titlebar_texture);
		}
		free(win->cfg->title);
	}
	free(win->cfg);
	free(win);
	window_count--;
}

int window_contains_point(const Window* win, int logical_x, int logical_y) {
	if (!win || !win->cfg || win->cfg->hidden) return 0;
	return point_in_rect(logical_x, logical_y, &win->cfg->rect);
}

int window_get_scrollable_zone_rect(const Window* win, SDL_Rect* out_rect) {
	if (!win || !win->cfg || !out_rect) return EXIT_FAILURE;
	return window_scrollable_zone_rect_from_cfg(win->cfg, out_rect);
}

void window_handle_event(AppContext* ctx, Window* win, SDL_Event* event) {
	if (!ctx || !win || !win->cfg || !event) return;

	WindowConfig* cfg = win->cfg;
	if (cfg->hidden) return;

	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
		int mx = event->button.x;
		int my = event->button.y;

		/*
		 * Autoriser le drag depuis n'importe quelle zone de la fenêtre
		 * (pas seulement la barre de titre) sauf si un champ de saisie
		 * texte est actif globalement (SDL_IsTextInputActive()).
		 * Cela évite d'interférer avec la saisie dans les inputs.
		 */
		if (cfg->movable && (point_in_rect(mx, my, &cfg->rect) && !SDL_IsTextInputActive())) {
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
	} else if (event->type == SDL_MOUSEWHEEL) {
		if (!cfg->scrollable) return;

		int raw_mx = 0;
		int raw_my = 0;
		SDL_GetMouseState(&raw_mx, &raw_my);

		int mx = raw_mx;
		int my = raw_my;
		window_to_logical_coords(ctx, raw_mx, raw_my, &mx, &my);

		SDL_Rect zone = {0};
		if (window_scrollable_zone_rect_from_cfg(cfg, &zone) != EXIT_SUCCESS) return;
		if (!point_in_rect(mx, my, &zone)) return;

		int wheel_y = event->wheel.y;
		if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
			wheel_y = -wheel_y;
		}
		if (wheel_y == 0) return;

		cfg->scroll_offset += wheel_y * cfg->scroll_step;
		window_normalize_scroll_cfg(cfg);
	} else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
		cfg->dragging = 0;
	}
}

void window_render(SDL_Renderer* renderer, const Window* win) {
	if (!renderer || !win || !win->cfg || win->cfg->hidden) return;

	const WindowConfig* cfg = win->cfg;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	if (cfg->window_texture) {
		SDL_RenderCopy(renderer, cfg->window_texture, NULL, &cfg->rect);
	} else {
		SDL_SetRenderDrawColor(renderer, cfg->bg_color.r, cfg->bg_color.g, cfg->bg_color.b, cfg->bg_color.a);
		SDL_RenderFillRect(renderer, &cfg->rect);
	}

	if (cfg->titlebar_h > 0) {
		SDL_Rect titlebar = {
			.x = cfg->rect.x,
			.y = cfg->rect.y,
			.w = cfg->rect.w,
			.h = cfg->titlebar_h
		};
		if (cfg->titlebar_texture) {
			SDL_RenderCopy(renderer, cfg->titlebar_texture, NULL, &titlebar);
		} else {
			SDL_SetRenderDrawColor(renderer, cfg->titlebar_color.r, cfg->titlebar_color.g, cfg->titlebar_color.b, cfg->titlebar_color.a);
			SDL_RenderFillRect(renderer, &titlebar);
		}
		window_render_title(renderer, cfg);
	}

	if (!cfg->window_texture) {
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
		case WIN_CFG_WINDOW_TEXTURE: cfg->window_texture = (SDL_Texture*)value; break;
		case WIN_CFG_TITLEBAR_TEXTURE: cfg->titlebar_texture = (SDL_Texture*)value; break;
		case WIN_CFG_TITLE:
			if (window_set_title(cfg, (const char*)value) != EXIT_SUCCESS) return EXIT_FAILURE;
			break;
		case WIN_CFG_BORDER_THICKNESS: cfg->border_thickness = (int)value; break;
		case WIN_CFG_TITLEBAR_H: cfg->titlebar_h = (int)value; break;
		case WIN_CFG_RECT: cfg->rect = *((SDL_Rect*)value); break;
		case WIN_CFG_DRAGGING: cfg->dragging = ((int)value != 0); break;
		case WIN_CFG_DRAG_OFFSET_X: cfg->drag_offset_x = (int)value; break;
		case WIN_CFG_DRAG_OFFSET_Y: cfg->drag_offset_y = (int)value; break;
		case WIN_CFG_SCROLLABLE: cfg->scrollable = ((int)value != 0); break;
		case WIN_CFG_SCROLL_X1: cfg->scroll_x1 = (int)value; break;
		case WIN_CFG_SCROLL_Y1: cfg->scroll_y1 = (int)value; break;
		case WIN_CFG_SCROLL_X2: cfg->scroll_x2 = (int)value; break;
		case WIN_CFG_SCROLL_Y2: cfg->scroll_y2 = (int)value; break;
		case WIN_CFG_SCROLL_OFFSET: cfg->scroll_offset = (int)value; break;
		case WIN_CFG_SCROLL_MIN: cfg->scroll_min = (int)value; break;
		case WIN_CFG_SCROLL_MAX: cfg->scroll_max = (int)value; break;
		case WIN_CFG_SCROLL_STEP: cfg->scroll_step = (int)value; break;
		default:
			return EXIT_FAILURE;
	}

	if (need_rect_refresh) {
		window_update_rect(win);
	} else {
		window_normalize_scroll_cfg(cfg);
	}

	return EXIT_SUCCESS;
}

void window_content_to_logical(const Window* win, int rel_x, int rel_y, int* out_logical_x, int* out_logical_y) {
	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return;

	if (out_logical_x) *out_logical_x = (origin_x + rel_x) - (WIN_WIDTH / 2);
	if (out_logical_y) *out_logical_y = (WIN_HEIGHT / 2) - (origin_y + rel_y);
}

int window_place_button(const Window* win, Button* button, int rel_x, int rel_y) {
	if (!win || !win->cfg || !button || !button->cfg) return EXIT_FAILURE;

	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return EXIT_FAILURE;

	/* rel_x/rel_y: coordonnées relatives au centre de la fenêtre
	 * rel_x positif = droite, rel_y positif = haut
	 * On centre le bouton sur la position calculée */
	int screen_x = origin_x + rel_x - (button->cfg->rect.w / 2);
	int screen_y = origin_y - rel_y - (button->cfg->rect.h / 2);

	button->cfg->x = screen_x;
	button->cfg->y = screen_y;
	button->cfg->rect.x = screen_x;
	button->cfg->rect.y = screen_y;

	if (button->cfg->is_text) {
		button->cfg->text_rect.x = screen_x + (button->cfg->rect.w - button->cfg->text_rect.w) / 2;
		button->cfg->text_rect.y = screen_y + (button->cfg->rect.h - button->cfg->text_rect.h) / 2;
	}

	return EXIT_SUCCESS;
}

int window_place_input(const Window* win, Input* in, int rel_x, int rel_y) {
	if (!win || !win->cfg || !in || !in->cfg) return EXIT_FAILURE;

	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return EXIT_FAILURE;

	/* rel_x/rel_y: coordonnées relatives au centre de la fenêtre
	 * rel_x positif = droite, rel_y positif = haut
	 * On centre l'input sur la position calculée */
	int screen_x = origin_x + rel_x - (in->cfg->rect.w / 2);
	int screen_y = origin_y - rel_y - (in->cfg->rect.h / 2);

	in->cfg->x = screen_x;
	in->cfg->y = screen_y;
	in->cfg->rect.x = screen_x;
	in->cfg->rect.y = screen_y;

	return EXIT_SUCCESS;
}

int window_place_text(const Window* win, Text* text, int rel_x, int rel_y) {
	if (!win || !win->cfg || !text || !text->texture) return EXIT_FAILURE;

	int tex_w = 0;
	int tex_h = 0;
	if (SDL_QueryTexture(text->texture, NULL, NULL, &tex_w, &tex_h) != 0) return EXIT_FAILURE;

	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return EXIT_FAILURE;

	/* rel_x/rel_y: coordonnées relatives au centre de la fenêtre
	 * rel_x positif = droite, rel_y positif = haut
	 * On centre le texte sur la position calculée */
	int screen_x = origin_x + rel_x;
	int screen_y = origin_y - rel_y;

	/* Convertir en coordonnées logiques pour update_text_position */
	int logical_x = screen_x - (WIN_WIDTH / 2);
	int logical_y = (WIN_HEIGHT / 2) - screen_y;

	update_text_position(text, logical_x, logical_y);
	return EXIT_SUCCESS;
}

int window_display_image(SDL_Renderer* renderer, const Window* win, SDL_Texture* texture, int rel_x, int rel_y, float size_factor, double angle, SDL_RendererFlip flip, float ratio, Uint8 opacity) {
	if (!renderer || !win || !texture) return EXIT_FAILURE;

	int og_w = 0;
	int og_h = 0;
	if (SDL_QueryTexture(texture, NULL, NULL, &og_w, &og_h) != 0) return EXIT_FAILURE;

	if (size_factor <= 0.0f) return EXIT_FAILURE;

	int base_w = (ratio != 1.0f) ? (int)(og_w * ratio) : og_w;
	if (base_w <= 0) return EXIT_FAILURE;

	int draw_w = (int)(base_w * size_factor);
	int draw_h = (int)(og_h * size_factor);
	if (draw_w <= 0 || draw_h <= 0) return EXIT_FAILURE;

	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return EXIT_FAILURE;

	/* rel_x/rel_y: coordonnées relatives au centre de la fenêtre
	 * rel_x positif = droite, rel_y positif = haut
	 * L'image sera centrée sur cette position */
	int screen_x = origin_x + rel_x;
	int screen_y = origin_y - rel_y;

	/* Convertir en coordonnées logiques pour display_image */
	int logical_x = screen_x - (WIN_WIDTH / 2);
	int logical_y = (WIN_HEIGHT / 2) - screen_y;

	return display_image(renderer, texture, logical_x, logical_y, size_factor, angle, flip, ratio, opacity);
}
