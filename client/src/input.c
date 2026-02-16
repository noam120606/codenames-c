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
    in->font_path = font_path;
    in->font_size = font_size;
    in->on_submit = NULL;
    return in;
}

void input_destroy(Input* in) {
    if (!in) return;
    if (in->text) free(in->text);
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

    /* background */
    SDL_SetRenderDrawColor(renderer, in->bg_color.r, in->bg_color.g, in->bg_color.b, in->bg_color.a);
    SDL_RenderFillRect(renderer, &in->rect);

    /* border */
    SDL_SetRenderDrawColor(renderer, in->border_color.r, in->border_color.g, in->border_color.b, in->border_color.a);
    SDL_RenderDrawRect(renderer, &in->rect);

    /* render text using TTF */
    if (in->font_path && in->font_size > 0) {
        TTF_Font* font = TTF_OpenFont(in->font_path, in->font_size);
        if (font) {
            SDL_Surface* surf = TTF_RenderText_Blended(font, in->text, in->text_color);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int tw = 0, th = 0;
                    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
                    int padding = 8;
                    SDL_Rect dst = { in->rect.x + padding, in->rect.y + (in->rect.h - th) / 2, tw, th };

                    /* If text wider than input, clip source width */
                    SDL_Rect src = {0, 0, tw, th};
                    if (dst.w > in->rect.w - padding*2) {
                        src.w = (in->rect.w - padding*2) * ((float)tw / dst.w);
                        dst.w = in->rect.w - padding*2;
                    }

                    SDL_RenderCopy(renderer, tex, &src, &dst);

                    /* draw cursor if focused */
                    if (in->focused) {
                        /* compute width of text up to cursor */
                        char before_cursor[512];
                        int n = in->cursor_pos;
                        if (n > 0) {
                            if (n >= (int)sizeof(before_cursor)) n = sizeof(before_cursor)-1;
                            memcpy(before_cursor, in->text, n);
                            before_cursor[n] = '\0';
                        } else before_cursor[0] = '\0';

                        int cx = 0, cy = 0;
                        TTF_SizeText(font, before_cursor, &cx, &cy);
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

                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
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
