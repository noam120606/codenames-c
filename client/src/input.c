/* Input widget implementation */
#include "../lib/all.h"
#include "../lib/input.h"

Input* input_create(int x, int y, int w, int h, const char* font_path, int font_size, int maxlen) {
    Input* in = (Input*)malloc(sizeof(Input));
    if (!in) return NULL;
    in->rect.x = x;
    in->rect.y = y;
    in->rect.w = w;
    in->rect.h = h;
    in->maxlen = (maxlen > 0) ? maxlen : INPUT_DEFAULT_MAX;
    in->text = (char*)calloc(in->maxlen + 1, 1);
    in->len = 0;
    in->cursor_pos = 0;
    in->focused = 0;
    in->submitted = 0;
    in->bg_color = (SDL_Color){255, 255, 255, 255};
    in->border_color = (SDL_Color){0, 0, 0, 255};
    in->text_color = (SDL_Color){0, 0, 0, 255};
    in->bg_texture = NULL;
    in->padding = 8;
    in->font_path = font_path;
    in->font_size = font_size;
    in->on_submit = NULL;
    return in;
}

void input_destroy(Input* in) {
    if (!in) return;
    if (in->text) free(in->text);
    if (in->bg_texture) free_image(in->bg_texture);
    free(in);
}

static int point_in_rect(int x, int y, SDL_Rect* r) {
    return x >= r->x && x <= (r->x + r->w) && y >= r->y && y <= (r->y + r->h);
}

void input_handle_event(Input* in, SDL_Event* e) {
    if (!in || !e) return;

    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int mx = e->button.x;
        int my = e->button.y;
        if (point_in_rect(mx, my, &in->rect)) {
            in->focused = 1;
            SDL_StartTextInput();
            /* place cursor at end */
            in->cursor_pos = in->len;
        } else {
            if (in->focused) {
                in->focused = 0;
                SDL_StopTextInput();
            }
        }
    }

    if (!in->focused) return;

    if (e->type == SDL_TEXTINPUT) {
        const char* txt = e->text.text;
        int add = strlen(txt);
        if (in->len + add > in->maxlen) return;
        /* insert at cursor_pos */
        memmove(in->text + in->cursor_pos + add, in->text + in->cursor_pos, in->len - in->cursor_pos + 1);
        memcpy(in->text + in->cursor_pos, txt, add);
        in->cursor_pos += add;
        in->len += add;
    }

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_BACKSPACE) {
            if (in->cursor_pos > 0 && in->len > 0) {
                memmove(in->text + in->cursor_pos - 1, in->text + in->cursor_pos, in->len - in->cursor_pos + 1);
                in->cursor_pos--;
                in->len--;
            }
        } else if (k == SDLK_LEFT) {
            if (in->cursor_pos > 0) in->cursor_pos--;
        } else if (k == SDLK_RIGHT) {
            if (in->cursor_pos < in->len) in->cursor_pos++;
        } else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            in->submitted = 1;
            in->focused = 0;
            SDL_StopTextInput();
            if (in->on_submit) in->on_submit(in->text);
        }
    }
}

void input_render(SDL_Renderer* renderer, Input* in) {
    if (!renderer || !in) return;

    /* background: image if present else color */
    if (in->bg_texture) {
        SDL_RenderCopy(renderer, in->bg_texture, NULL, &in->rect);
    } else {
        SDL_SetRenderDrawColor(renderer, in->bg_color.r, in->bg_color.g, in->bg_color.b, in->bg_color.a);
        SDL_RenderFillRect(renderer, &in->rect);
    }

    /* border: draw only when no background texture is set */
    if (!in->bg_texture) {
        SDL_SetRenderDrawColor(renderer, in->border_color.r, in->border_color.g, in->border_color.b, in->border_color.a);
        SDL_RenderDrawRect(renderer, &in->rect);
    }

    /* render text using TTF */
    if (in->font_path && in->font_size > 0) {
        TTF_Font* font = TTF_OpenFont(in->font_path, in->font_size);
        if (font) {
            /* compute width of text up to cursor so we can draw cursor even when no
             * texture is created (empty string case) */
            char before_cursor[512];
            int n = in->cursor_pos;
            if (n > 0) {
                if (n >= (int)sizeof(before_cursor)) n = sizeof(before_cursor)-1;
                memcpy(before_cursor, in->text, n);
                before_cursor[n] = '\0';
            } else before_cursor[0] = '\0';

            int cx = 0, cy = 0;
            TTF_SizeText(font, before_cursor, &cx, &cy);
            int padding = in->padding;

            SDL_Surface* surf = NULL;
            SDL_Texture* tex = NULL;
            surf = TTF_RenderText_Blended(font, in->text, in->text_color);
            if (surf) {
                tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int tw = 0, th = 0;
                    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
                    SDL_Rect dst = { in->rect.x + padding, in->rect.y + (in->rect.h - th) / 2, tw, th };

                    /* If text wider than input, clip source width */
                    SDL_Rect src = {0, 0, tw, th};
                    if (dst.w > in->rect.w - padding*2) {
                        src.w = (in->rect.w - padding*2) * ((float)tw / dst.w);
                        dst.w = in->rect.w - padding*2;
                    }

                    SDL_RenderCopy(renderer, tex, &src, &dst);
                }
            }

            /* draw cursor if focused (draw regardless of surf/tex) */
            if (in->focused) {
                int cursor_x = in->rect.x + padding + cx;
                /* ensure cursor stays inside rect */
                if (cursor_x > in->rect.x + in->rect.w - padding) cursor_x = in->rect.x + in->rect.w - padding;

                /* blink */
                Uint32 t = SDL_GetTicks();
                if ((t / 500) % 2 == 0) {
                    SDL_SetRenderDrawColor(renderer, in->text_color.r, in->text_color.g, in->text_color.b, in->text_color.a);
                    SDL_Rect cur = { cursor_x, in->rect.y + 6, 2, in->rect.h - 12 };
                    SDL_RenderFillRect(renderer, &cur);
                }
            }

            if (tex) SDL_DestroyTexture(tex);
            if (surf) SDL_FreeSurface(surf);
            TTF_CloseFont(font);
        }
    }
}

const char* input_get_text(Input* in) {
    if (!in) return NULL;
    return in->text;
}

int input_is_submitted(Input* in) {
    if (!in) return 0;
    return in->submitted;
}

void input_clear_submitted(Input* in) {
    if (!in) return;
    in->submitted = 0;
}

void input_set_text(Input* in, const char* text) {
    if (!in || !text) return;
    strncpy(in->text, text, in->maxlen);
    in->text[in->maxlen] = '\0';
    in->len = strlen(in->text);
    in->cursor_pos = in->len;
}

void input_set_on_submit(Input* in, void (*cb)(const char*)) {
    if (!in) return;
    in->on_submit = cb;
}

int input_set_bg(Input* in, SDL_Renderer* renderer, const char* path, int padding) {
    if (!in || !renderer || !path) return EXIT_FAILURE;
    if (in->bg_texture) {
        free_image(in->bg_texture);
        in->bg_texture = NULL;
    }
    SDL_Texture* tex = load_image(renderer, path);
    if (!tex) return EXIT_FAILURE;
    /* If caller provided padding >= 0, use it; otherwise adapt to texture */
    if (padding >= 0) {
        in->padding = padding;
    } else {
        int tw = 0, th = 0;
        if (SDL_QueryTexture(tex, NULL, NULL, &tw, &th) == 0) {
            int p = in->rect.h / 3;
            if (p < 6) p = 6;
            if (tw > 0 && tw < in->rect.w) p += 4;
            in->padding = p;
        }
    }
    in->bg_texture = tex;
    return EXIT_SUCCESS;
}

void input_clear_bg(Input* in) {
    if (!in) return;
    if (in->bg_texture) {
        free_image(in->bg_texture);
        in->bg_texture = NULL;
    }
    in->padding = 8;
}

void input_set_padding(Input* in, int padding) {
    if (!in) return;
    if (padding < 0) padding = 0;
    in->padding = padding;
}
