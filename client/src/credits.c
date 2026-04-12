#include "../lib/all.h"

#define CREDITS_FILE_PATH "assets/misc/credits.txt"
#define CREDITS_TEXT_WRAP_PADDING 30
#define CREDITS_TEXT_FONT_SIZE 32
#define CREDITS_TEXT_LINE_ADVANCE_FACTOR 1.2f
#define CREDITS_TEXT_TOP_PADDING 24
#define CREDITS_TEXT_BOTTOM_PADDING 120
#define CREDITS_TEXT_SIDE_PADDING 28
#define CREDITS_SCROLL_SPEED_PX_PER_SEC 70.0f
#define CREDITS_SCROLL_RESUME_DELAY_MS 4000U
#define CREDITS_SCROLL_WHEEL_STEP 60.0f
#define CREDITS_CLOSE_BUTTON_W 440
#define CREDITS_CLOSE_BUTTON_H 72
#define CREDITS_RESTART_BUTTON_W 250
#define CREDITS_RESTART_BUTTON_H 50
#define CREDITS_ACTION_BOTTOM_MARGIN 68
#define CREDITS_ACTION_LEFT_MARGIN 42

typedef struct CreditsState {
    Window* window;
    Button* btn_close;
    Button* btn_restart;
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

    if (g_credits.btn_restart && g_credits.btn_restart->cfg) {
        g_credits.btn_restart->cfg->is_hovered = 0;
        g_credits.btn_restart->cfg->is_pressed = 0;
    }
}

static void credits_request_full_restart(AppContext* context) {
    if (!context) return;

    context->restart_requested = 1;

    SDL_Event quit_event;
    SDL_zero(quit_event);
    quit_event.type = SDL_QUIT;
    (void)SDL_PushEvent(&quit_event);
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

    TTF_Font* font = TTF_OpenFont(FONT_LARABIE, CREDITS_TEXT_FONT_SIZE);
    if (!font) {
        return EXIT_FAILURE;
    }

    typedef struct CreditsRenderedLine {
        SDL_Surface* surface;
        int h;
        int advance;
    } CreditsRenderedLine;

    size_t line_count = 1;
    for (const char* p = text; *p; p++) {
        if (*p == '\n') line_count++;
    }

    CreditsRenderedLine* lines = (CreditsRenderedLine*)calloc(line_count, sizeof(CreditsRenderedLine));
    if (!lines) {
        TTF_CloseFont(font);
        return EXIT_FAILURE;
    }

    const char* cursor = text;
    int max_w = 1;
    int total_h = 0;
    int build_error = 0;

    for (size_t i = 0; i < line_count; i++) {
        const char* line_end = strchr(cursor, '\n');
        size_t raw_len = line_end ? (size_t)(line_end - cursor) : strlen(cursor);

        char* line_text = (char*)malloc(raw_len + 1);
        if (!line_text) {
            build_error = 1;
            break;
        }

        memcpy(line_text, cursor, raw_len);
        line_text[raw_len] = '\0';
        if (raw_len > 0 && line_text[raw_len - 1] == '\r') {
            line_text[raw_len - 1] = '\0';
            raw_len--;
        }

        if (raw_len == 0) {
            lines[i].surface = NULL;
            lines[i].h = TTF_FontHeight(font);
        } else {
            lines[i].surface = TTF_RenderUTF8_Blended_Wrapped(font, line_text, COL_WHITE, (Uint32)wrap_width);
            if (!lines[i].surface) {
                free(line_text);
                build_error = 1;
                break;
            }

            lines[i].h = lines[i].surface->h;
            if (lines[i].surface->w > max_w) {
                max_w = lines[i].surface->w;
            }
        }

        lines[i].advance = (int)lroundf((double)((float)lines[i].h * CREDITS_TEXT_LINE_ADVANCE_FACTOR));
        if (lines[i].advance < 1) lines[i].advance = 1;

        total_h += (i + 1 < line_count) ? lines[i].advance : lines[i].h;
        free(line_text);

        if (!line_end) break;
        cursor = line_end + 1;
    }

    if (total_h <= 0) {
        total_h = TTF_FontHeight(font);
        if (total_h <= 0) total_h = 1;
    }

    SDL_Surface* composed = NULL;

    if (!build_error) {
        composed = SDL_CreateRGBSurfaceWithFormat(0, max_w, total_h, 32, SDL_PIXELFORMAT_RGBA32);
        if (!composed) {
            build_error = 1;
        }
    }

    if (!build_error) {
        SDL_SetSurfaceBlendMode(composed, SDL_BLENDMODE_BLEND);
        SDL_FillRect(composed, NULL, SDL_MapRGBA(composed->format, 0, 0, 0, 0));

        int y = 0;
        for (size_t i = 0; i < line_count; i++) {
            if (lines[i].surface) {
                SDL_Rect dst = {
                    .x = (max_w - lines[i].surface->w) / 2,
                    .y = y,
                    .w = lines[i].surface->w,
                    .h = lines[i].surface->h
                };
                SDL_BlitSurface(lines[i].surface, NULL, composed, &dst);
            }
            y += (i + 1 < line_count) ? lines[i].advance : lines[i].h;
        }

        g_credits.text_texture = SDL_CreateTextureFromSurface(renderer, composed);
        if (!g_credits.text_texture) {
            build_error = 1;
        }
    }

    if (!build_error) {
        g_credits.text_w = max_w;
        g_credits.text_h = total_h;
    }

    if (composed) {
        SDL_FreeSurface(composed);
    }

    for (size_t i = 0; i < line_count; i++) {
        if (lines[i].surface) {
            SDL_FreeSurface(lines[i].surface);
            lines[i].surface = NULL;
        }
    }
    free(lines);
    TTF_CloseFont(font);

    if (build_error) {
        if (g_credits.text_texture) {
            SDL_DestroyTexture(g_credits.text_texture);
            g_credits.text_texture = NULL;
        }
        g_credits.text_w = 0;
        g_credits.text_h = 0;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static ButtonReturn credits_action_button_callback(AppContext* context, Button* button) {
    if (!button) return BTN_NONE;

    if (button == g_credits.btn_close) {
        g_credits.active = 0;
        credits_reset_button_state();
    } else if (button == g_credits.btn_restart) {
        g_credits.active = 0;
        credits_reset_button_state();
        credits_request_full_restart(context);
    }

    return BTN_NONE;
}

static void credits_place_action_buttons(void) {
    if (!g_credits.window || !g_credits.window->cfg) return;

    int rel_y = -((g_credits.window->cfg->h / 2) - CREDITS_ACTION_BOTTOM_MARGIN);

    if (g_credits.btn_close) {
        (void)window_place_button(g_credits.window, g_credits.btn_close, 0, rel_y);
    }

    if (g_credits.btn_restart && g_credits.btn_restart->cfg) {
        int rel_x = -((g_credits.window->cfg->w / 2) - (g_credits.btn_restart->cfg->w / 2) - CREDITS_ACTION_LEFT_MARGIN);
        (void)window_place_button(g_credits.window, g_credits.btn_restart, rel_x, rel_y);
    }
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

    cfg_close->w = CREDITS_CLOSE_BUTTON_W;
    cfg_close->h = CREDITS_CLOSE_BUTTON_H;
    cfg_close->font_path = FONT_LARABIE;
    cfg_close->color = COL_WHITE;
    cfg_close->text = "Fermer les credits";
    cfg_close->callback = credits_action_button_callback;

    g_credits.btn_close = button_create(context->renderer, BTN_CREDITS_CLOSE, cfg_close);
    free(cfg_close);

    if (!g_credits.btn_close) {
        credits_free();
        return EXIT_FAILURE;
    }

    ButtonConfig* cfg_restart = button_config_init();
    if (!cfg_restart) {
        credits_free();
        return EXIT_FAILURE;
    }

    cfg_restart->w = CREDITS_RESTART_BUTTON_W;
    cfg_restart->h = CREDITS_RESTART_BUTTON_H;
    cfg_restart->font_path = FONT_LARABIE;
    cfg_restart->color = COL_WHITE;
    cfg_restart->text = "Relancer le jeu";
    cfg_restart->callback = credits_action_button_callback;

    g_credits.btn_restart = button_create(context->renderer, BTN_CREDITS_RESTART_GAME, cfg_restart);
    free(cfg_restart);

    if (!g_credits.btn_restart) {
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

    credits_place_action_buttons();
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

    credits_place_action_buttons();
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

    if (g_credits.btn_restart) {
        (void)button_handle_event(context, g_credits.btn_restart, e);
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

    credits_place_action_buttons();

    if (g_credits.btn_close) {
        button_render(context->renderer, g_credits.btn_close);
    }

    if (g_credits.btn_restart) {
        button_render(context->renderer, g_credits.btn_restart);
    }
}

int credits_free(void) {
    if (g_credits.btn_close) {
        button_destroy(g_credits.btn_close);
        g_credits.btn_close = NULL;
    }

    if (g_credits.btn_restart) {
        button_destroy(g_credits.btn_restart);
        g_credits.btn_restart = NULL;
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
