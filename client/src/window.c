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

static int window_content_origin_screen(const Window* win, int* out_x, int* out_y) {
	if (!win || !win->cfg) return EXIT_FAILURE;
	if (out_x) *out_x = win->cfg->rect.x;
	if (out_y) *out_y = win->cfg->rect.y + clamp_non_negative(win->cfg->titlebar_h);
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
	cfg->title = NULL;
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
		const char* incoming_title = cfg_in->title;
		*(win->cfg) = *cfg_in;
		win->cfg->title = NULL;
		if (window_set_title(win->cfg, incoming_title) != EXIT_SUCCESS) {
			window_destroy(win);
			return NULL;
		}
	}

	window_update_rect(win);
	return win;
}

void window_destroy(Window* win) {
	if (!win) return;
	if (win->cfg) {
		free(win->cfg->title);
	}
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
		window_render_title(renderer, cfg);
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
		case WIN_CFG_TITLE:
			if (window_set_title(cfg, (const char*)value) != EXIT_SUCCESS) return EXIT_FAILURE;
			break;
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

	int x = origin_x + rel_x;
	int y = origin_y + rel_y;

	button->cfg->x = x;
	button->cfg->y = y;
	button->cfg->rect.x = x;
	button->cfg->rect.y = y;

	if (button->cfg->is_text) {
		button->cfg->text_rect.x = x + (button->cfg->rect.w - button->cfg->text_rect.w) / 2;
		button->cfg->text_rect.y = y + (button->cfg->rect.h - button->cfg->text_rect.h) / 2;
	}

	return EXIT_SUCCESS;
}

int window_place_input(const Window* win, Input* in, int rel_x, int rel_y) {
	if (!win || !win->cfg || !in || !in->cfg) return EXIT_FAILURE;

	int origin_x = 0;
	int origin_y = 0;
	if (window_content_origin_screen(win, &origin_x, &origin_y) != EXIT_SUCCESS) return EXIT_FAILURE;

	in->cfg->x = origin_x + rel_x;
	in->cfg->y = origin_y + rel_y;
	in->cfg->rect.x = in->cfg->x;
	in->cfg->rect.y = in->cfg->y;

	return EXIT_SUCCESS;
}

int window_place_text(const Window* win, Text* text, int rel_x, int rel_y) {
	if (!win || !win->cfg || !text || !text->texture) return EXIT_FAILURE;

	int tex_w = 0;
	int tex_h = 0;
	if (SDL_QueryTexture(text->texture, NULL, NULL, &tex_w, &tex_h) != 0) return EXIT_FAILURE;

	int logical_x = 0;
	int logical_y = 0;
	window_content_to_logical(win, rel_x, rel_y, &logical_x, &logical_y);

	/* display_text centre le texte autour de cfg.x/cfg.y */
	update_text_position(text, logical_x + (tex_w / 2), logical_y - (tex_h / 2));
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

	int logical_x = 0;
	int logical_y = 0;
	window_content_to_logical(win, rel_x, rel_y, &logical_x, &logical_y);

	int center_logical_x = logical_x + (draw_w / 2);
	int center_logical_y = logical_y - (draw_h / 2);

	return display_image(renderer, texture, center_logical_x, center_logical_y, size_factor, angle, flip, ratio, opacity);
}
