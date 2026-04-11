#include "../lib/all.h"

#define CREDITS_FILE_PATH "assets/misc/credits.txt"
#define CREDITS_TEXT_WRAP_PADDING 42
#define CREDITS_TEXT_TOP_PADDING 24
#define CREDITS_TEXT_BOTTOM_PADDING 120
#define CREDITS_TEXT_SIDE_PADDING 28
#define CREDITS_SCROLL_SPEED_PX_PER_SEC 70.0f
#define CREDITS_SCROLL_RESUME_DELAY_MS 5000U
#define CREDITS_SCROLL_WHEEL_STEP 68.0f

typedef struct CreditsState {
    Window* window;
    Button* btn_close;
    SDL_Texture* text_texture;
    char* text_content;
    int text_w;
    int text_h;
    float scroll_offset;
    int auto_scroll_paused;
    Uint32 last_tick_ms;
    Uint32 last_scroll_input_ms;
    int active;
    int initialized;
} CreditsState;

static CreditsState g_credits = {0};

static float credits_clamp_float(float value, float min_value, float max_value) {
    if (min_value > max_value) {
        float tmp = min_value;
        min_value = max_value;
        max_value = tmp;
    }

    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static int credits_get_content_rect(SDL_Rect* out_rect) {
    if (!out_rect || !g_credits.window || !g_credits.window->cfg) return EXIT_FAILURE;

    const WindowConfig* cfg = g_credits.window->cfg;

    SDL_Rect r = {
        .x = cfg->rect.x + CREDITS_TEXT_SIDE_PADDING,
        .y = cfg->rect.y + cfg->titlebar_h + CREDITS_TEXT_TOP_PADDING,
        .w = cfg->rect.w - (2 * CREDITS_TEXT_SIDE_PADDING),
        .h = cfg->rect.h - cfg->titlebar_h - CREDITS_TEXT_TOP_PADDING - CREDITS_TEXT_BOTTOM_PADDING
    };

    if (r.w <= 0 || r.h <= 0) return EXIT_FAILURE;

    *out_rect = r;
    return EXIT_SUCCESS;
}

static float credits_max_scroll_for_rect(const SDL_Rect* content_rect) {
    if (!content_rect) return 0.0f;
    if (g_credits.text_h <= content_rect->h) return 0.0f;
    return (float)(g_credits.text_h - content_rect->h);
}

static void credits_clamp_scroll_offset(void) {
    SDL_Rect content_rect = {0};
    if (credits_get_content_rect(&content_rect) != EXIT_SUCCESS) {
        g_credits.scroll_offset = 0.0f;
        return;
    }

    float max_scroll = credits_max_scroll_for_rect(&content_rect);
    g_credits.scroll_offset = credits_clamp_float(g_credits.scroll_offset, 0.0f, max_scroll);
}

static void credits_reset_button_state(void) {
    if (g_credits.btn_close && g_credits.btn_close->cfg) {
        g_credits.btn_close->cfg->is_hovered = 0;
        g_credits.btn_close->cfg->is_pressed = 0;
    }
}

static int credits_load_file_content(void) {
    const char* fallback_text = "Aucun credit disponible.";

    free(g_credits.text_content);
    g_credits.text_content = NULL;

    FILE* f = fopen(CREDITS_FILE_PATH, "rb");
    if (!f) {
        g_credits.text_content = strdup(fallback_text);
        return g_credits.text_content ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        g_credits.text_content = strdup(fallback_text);
        return g_credits.text_content ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    long file_size = ftell(f);
    if (file_size < 0) {
        fclose(f);
        g_credits.text_content = strdup(fallback_text);
        return g_credits.text_content ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        g_credits.text_content = strdup(fallback_text);
        return g_credits.text_content ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    size_t alloc_size = (size_t)file_size + 1;
    char* buffer = (char*)malloc(alloc_size);
    if (!buffer) {
        fclose(f);
        return EXIT_FAILURE;
    }

    size_t read_size = fread(buffer, 1, (size_t)file_size, f);
    fclose(f);

    buffer[read_size] = '\0';

    if (read_size == 0) {
        free(buffer);
        g_credits.text_content = strdup(fallback_text);
        return g_credits.text_content ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    g_credits.text_content = buffer;
    return EXIT_SUCCESS;
}

static int credits_build_text_texture(SDL_Renderer* renderer) {
    if (!renderer) return EXIT_FAILURE;

    if (g_credits.text_texture) {
        SDL_DestroyTexture(g_credits.text_texture);
        g_credits.text_texture = NULL;
    }

    g_credits.text_w = 0;
    g_credits.text_h = 0;

    SDL_Rect content_rect = {0};
    if (credits_get_content_rect(&content_rect) != EXIT_SUCCESS) return EXIT_FAILURE;

    int wrap_width = content_rect.w - CREDITS_TEXT_WRAP_PADDING;
    if (wrap_width <= 1) wrap_width = content_rect.w;
    if (wrap_width <= 1) wrap_width = 1;

    const char* text = (g_credits.text_content && g_credits.text_content[0] != '\0')
        ? g_credits.text_content
        : "Aucun credit disponible.";

    TTF_Font* font = TTF_OpenFont(FONT_LARABIE, 36);
    if (!font) {
        return EXIT_FAILURE;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text, COL_WHITE, (Uint32)wrap_width);
    if (!surface) {
        TTF_CloseFont(font);
        return EXIT_FAILURE;
    }

    g_credits.text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!g_credits.text_texture) {
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return EXIT_FAILURE;
    }

    g_credits.text_w = surface->w;
    g_credits.text_h = surface->h;

    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return EXIT_SUCCESS;
}

static int credits_close_button_callback(AppContext* context, Button* button) {
    (void)context;
    if (!button) return BTN_NONE;

    if (button == g_credits.btn_close) {
        g_credits.active = 0;
        credits_reset_button_state();
    }

    return BTN_NONE;
}

static void credits_place_close_button(void) {
    if (!g_credits.window || !g_credits.window->cfg || !g_credits.btn_close) return;

    int rel_y = -((g_credits.window->cfg->h / 2) - 68);
    (void)window_place_button(g_credits.window, g_credits.btn_close, 0, rel_y);
}

static void credits_update_auto_scroll(void) {
    if (!g_credits.active) return;

    Uint32 now = SDL_GetTicks();
    if (g_credits.last_tick_ms == 0) {
        g_credits.last_tick_ms = now;
        return;
    }

    if (g_credits.auto_scroll_paused) {
        if (now - g_credits.last_scroll_input_ms >= CREDITS_SCROLL_RESUME_DELAY_MS) {
            g_credits.auto_scroll_paused = 0;
        }
    }

    if (!g_credits.auto_scroll_paused) {
        float dt = (float)(now - g_credits.last_tick_ms) / 1000.0f;
        g_credits.scroll_offset += CREDITS_SCROLL_SPEED_PX_PER_SEC * dt;
        credits_clamp_scroll_offset();
    }

    g_credits.last_tick_ms = now;
}

int credits_init(AppContext* context) {
    if (!context || !context->renderer) return EXIT_FAILURE;
    if (g_credits.initialized) return EXIT_SUCCESS;

    WindowConfig* cfg_window = window_config_init();
    if (!cfg_window) return EXIT_FAILURE;

    cfg_window->x = 0;
    cfg_window->y = 0;
    cfg_window->w = 1450;
    cfg_window->h = 900;
    cfg_window->movable = 0;
    cfg_window->bg_color = (SDL_Color){24, 24, 24, 240};
    cfg_window->border_color = (SDL_Color){220, 220, 220, 255};
    cfg_window->titlebar_color = (SDL_Color){35, 80, 120, 255};
    cfg_window->titlebar_h = 52;
    cfg_window->border_thickness = 3;
    cfg_window->title = "Credits";

    g_credits.window = window_create(WINDOW_CREDITS, cfg_window);
    free(cfg_window);

    if (!g_credits.window) {
        credits_free();
        return EXIT_FAILURE;
    }

    ButtonConfig* cfg_close = button_config_init();
    if (!cfg_close) {
        credits_free();
        return EXIT_FAILURE;
    }

    cfg_close->w = 440;
    cfg_close->h = 72;
    cfg_close->font_path = FONT_LARABIE;
    cfg_close->color = COL_WHITE;
    cfg_close->text = "Fermer les credits";
    cfg_close->callback = credits_close_button_callback;

    g_credits.btn_close = button_create(context->renderer, BTN_CREDITS_CLOSE, cfg_close);
    free(cfg_close);

    if (!g_credits.btn_close) {
        credits_free();
        return EXIT_FAILURE;
    }

    if (credits_load_file_content() != EXIT_SUCCESS) {
        credits_free();
        return EXIT_FAILURE;
    }

    if (credits_build_text_texture(context->renderer) != EXIT_SUCCESS) {
        credits_free();
        return EXIT_FAILURE;
    }

    g_credits.scroll_offset = 0.0f;
    g_credits.auto_scroll_paused = 0;
    g_credits.last_tick_ms = 0;
    g_credits.last_scroll_input_ms = 0;
    g_credits.active = 0;
    g_credits.initialized = 1;

    credits_place_close_button();
    credits_clamp_scroll_offset();

    return EXIT_SUCCESS;
}

void credits_open(void) {
    if (!g_credits.initialized) return;

    g_credits.active = 1;
    g_credits.scroll_offset = 0.0f;
    g_credits.auto_scroll_paused = 0;
    g_credits.last_tick_ms = SDL_GetTicks();
    g_credits.last_scroll_input_ms = 0;

    credits_place_close_button();
    credits_reset_button_state();
}

int credits_is_active(void) {
    return g_credits.active != 0;
}

void credits_handle_event(AppContext* context, SDL_Event* e) {
    if (!g_credits.active || !e) return;

    if (g_credits.window) {
        window_handle_event(context, g_credits.window, e);
    }

    if (e->type == SDL_MOUSEWHEEL) {
        int wheel_y = e->wheel.y;
        if (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            wheel_y = -wheel_y;
        }

        if (wheel_y != 0) {
            g_credits.scroll_offset -= (float)wheel_y * CREDITS_SCROLL_WHEEL_STEP;
            credits_clamp_scroll_offset();
            g_credits.auto_scroll_paused = 1;
            g_credits.last_scroll_input_ms = SDL_GetTicks();
        }
    }

    if (g_credits.btn_close) {
        (void)button_handle_event(context, g_credits.btn_close, e);
    }
}

void credits_display(AppContext* context) {
    if (!context || !context->renderer || !g_credits.active) return;

    credits_update_auto_scroll();

    if (g_credits.window) {
        window_render(context->renderer, g_credits.window);
    }

    SDL_Rect content_rect = {0};
    if (credits_get_content_rect(&content_rect) != EXIT_SUCCESS) return;

    SDL_SetRenderDrawBlendMode(context->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(context->renderer, 15, 15, 15, 220);
    SDL_RenderFillRect(context->renderer, &content_rect);
    SDL_SetRenderDrawColor(context->renderer, 90, 90, 90, 255);
    SDL_RenderDrawRect(context->renderer, &content_rect);

    if (g_credits.text_texture) {
        SDL_Rect dst = {
            .x = content_rect.x + (content_rect.w - g_credits.text_w) / 2,
            .y = content_rect.y - (int)lroundf(g_credits.scroll_offset),
            .w = g_credits.text_w,
            .h = g_credits.text_h
        };

        SDL_RenderSetClipRect(context->renderer, &content_rect);
        SDL_RenderCopy(context->renderer, g_credits.text_texture, NULL, &dst);
        SDL_RenderSetClipRect(context->renderer, NULL);
    }

    credits_place_close_button();

    if (g_credits.btn_close) {
        button_render(context->renderer, g_credits.btn_close);
    }
}

int credits_free(void) {
    if (g_credits.btn_close) {
        button_destroy(g_credits.btn_close);
        g_credits.btn_close = NULL;
    }

    if (g_credits.window) {
        window_destroy(g_credits.window);
        g_credits.window = NULL;
    }

    if (g_credits.text_texture) {
        SDL_DestroyTexture(g_credits.text_texture);
        g_credits.text_texture = NULL;
    }

    free(g_credits.text_content);
    g_credits.text_content = NULL;

    g_credits.text_w = 0;
    g_credits.text_h = 0;
    g_credits.scroll_offset = 0.0f;
    g_credits.auto_scroll_paused = 0;
    g_credits.last_tick_ms = 0;
    g_credits.last_scroll_input_ms = 0;
    g_credits.active = 0;
    g_credits.initialized = 0;

    return EXIT_SUCCESS;
}
