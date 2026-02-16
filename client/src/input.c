/* Input widget implementation */
#include "../lib/all.h"

static void input_clear_selection_internal(Input* in) {
    if (!in) return;
    in->sel_start = 0;
    in->sel_len = 0;
}

static int input_has_selection(Input* in) {
    return in && in->sel_len > 0;
}

static void input_delete_selection(Input* in) {
    if (!in || in->sel_len <= 0) return;
    int s = in->sel_start;
    int l = in->sel_len;
    memmove(in->text + s, in->text + s + l, in->len - (s + l) + 1);
    in->len -= l;
    if (in->len < 0) in->len = 0;
    in->text[in->len] = '\0';
    in->cursor_pos = s;
    input_clear_selection_internal(in);
}

static int prev_word_pos(Input* in, int pos) {
    if (!in) return 0;
    int i = pos;
    while (i > 0 && isspace((unsigned char)in->text[i-1])) i--;
    while (i > 0 && !isspace((unsigned char)in->text[i-1])) i--;
    return i;
}

static int next_word_pos(Input* in, int pos) {
    if (!in) return 0;
    int i = pos;
    while (i < in->len && !isspace((unsigned char)in->text[i])) i++;
    while (i < in->len && isspace((unsigned char)in->text[i])) i++;
    return i;
}

Input* input_create(InputId id, int x, int y, int w, int h, const char* font_path, int font_size, const char** placeholders, int placeholder_count, const char* submitted_label, int maxlen) {
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
    in->id = id;
    in->sel_start = 0;
    in->sel_len = 0;
    in->padding = 8;
    in->font_path = font_path;
    in->font_size = font_size;
    in->on_submit = NULL;
    in->submitted_text = NULL;
    in->submitted_label = NULL;
    in->placeholders = NULL;
    in->placeholder_count = 0;
    in->placeholder_index = 0;
    in->placeholder_last_tick = SDL_GetTicks();
    if (placeholders && placeholder_count > 0) {
        in->placeholders = (char**)malloc(sizeof(char*) * placeholder_count);
        if (in->placeholders) {
            for (int i = 0; i < placeholder_count; ++i) {
                in->placeholders[i] = strdup(placeholders[i]);
            }
            in->placeholder_count = placeholder_count;
        }
    }
    if (submitted_label) {
        in->submitted_label = strdup(submitted_label);
    }
    return in;
}

void input_destroy(Input* in) {
    if (!in) return;
    if (in->text) free(in->text);
    if (in->bg_texture) free_image(in->bg_texture);
    if (in->submitted_text) free(in->submitted_text);
    if (in->submitted_label) free(in->submitted_label);
    if (in->placeholders) {
        for (int i = 0; i < in->placeholder_count; ++i) {
            if (in->placeholders[i]) free(in->placeholders[i]);
        }
        free(in->placeholders);
    }
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
        /* if there is a selection, replace it */
        if (input_has_selection(in)) {
            input_delete_selection(in);
        }
        /* insert at cursor_pos */
        memmove(in->text + in->cursor_pos + add, in->text + in->cursor_pos, in->len - in->cursor_pos + 1);
        memcpy(in->text + in->cursor_pos, txt, add);
        in->cursor_pos += add;
        in->len += add;
    }

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        SDL_Keymod mod = e->key.keysym.mod;
        /* Ctrl+A = select all */
        if ((mod & KMOD_CTRL) && (k == SDLK_a)) {
            in->sel_start = 0;
            in->sel_len = in->len;
            in->cursor_pos = in->len;
            return;
        }

        if (k == SDLK_BACKSPACE) {
            if (input_has_selection(in)) {
                input_delete_selection(in);
            } else if (mod & KMOD_CTRL) {
                int np = prev_word_pos(in, in->cursor_pos);
                int del = in->cursor_pos - np;
                if (del > 0) {
                    memmove(in->text + np, in->text + in->cursor_pos, in->len - in->cursor_pos + 1);
                    in->len -= del;
                    in->cursor_pos = np;
                    in->text[in->len] = '\0';
                }
            } else {
                if (in->cursor_pos > 0 && in->len > 0) {
                    memmove(in->text + in->cursor_pos - 1, in->text + in->cursor_pos, in->len - in->cursor_pos + 1);
                    in->cursor_pos--;
                    in->len--;
                }
            }
            input_clear_selection_internal(in);
        } else if (k == SDLK_DELETE) {
            if (input_has_selection(in)) {
                input_delete_selection(in);
            } else if (mod & KMOD_CTRL) {
                int np = next_word_pos(in, in->cursor_pos);
                int del = np - in->cursor_pos;
                if (del > 0) {
                    memmove(in->text + in->cursor_pos, in->text + np, in->len - np + 1);
                    in->len -= del;
                    in->text[in->len] = '\0';
                }
            } else {
                if (in->cursor_pos < in->len && in->len > 0) {
                    memmove(in->text + in->cursor_pos, in->text + in->cursor_pos + 1, in->len - in->cursor_pos);
                    in->len--;
                    in->text[in->len] = '\0';
                }
            }
            input_clear_selection_internal(in);
        } else if (k == SDLK_LEFT) {
            int newpos = in->cursor_pos;
            if (mod & KMOD_CTRL) newpos = prev_word_pos(in, in->cursor_pos);
            else if (in->cursor_pos > 0) newpos = in->cursor_pos - 1;

            if (mod & KMOD_SHIFT) {
                if (!input_has_selection(in)) in->sel_start = in->cursor_pos;
                in->cursor_pos = newpos;
                int s = in->sel_start;
                int epos = in->cursor_pos;
                if (epos < s) { in->sel_start = epos; in->sel_len = s - epos; }
                else { in->sel_start = s; in->sel_len = epos - s; }
            } else {
                in->cursor_pos = newpos;
                input_clear_selection_internal(in);
            }
        } else if (k == SDLK_RIGHT) {
            int newpos = in->cursor_pos;
            if (mod & KMOD_CTRL) newpos = next_word_pos(in, in->cursor_pos);
            else if (in->cursor_pos < in->len) newpos = in->cursor_pos + 1;

            if (mod & KMOD_SHIFT) {
                if (!input_has_selection(in)) in->sel_start = in->cursor_pos;
                in->cursor_pos = newpos;
                int s = in->sel_start;
                int epos = in->cursor_pos;
                if (epos < s) { in->sel_start = epos; in->sel_len = s - epos; }
                else { in->sel_start = s; in->sel_len = epos - s; }
            } else {
                in->cursor_pos = newpos;
                input_clear_selection_internal(in);
            }
        } else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            in->submitted = 1;
            /* store submitted text inside input */
            if (in->submitted_text) {
                free(in->submitted_text);
                in->submitted_text = NULL;
            }
            in->submitted_text = (char*)malloc(in->len + 1);
            if (in->submitted_text) strncpy(in->submitted_text, in->text, in->len + 1);

            /* call submit callback with the submitted text (use stored copy)
             * then clear the input contents so it appears empty */
            if (in->on_submit) in->on_submit(in->submitted_text);

            /* clear current text */
            if (in->text && in->maxlen > 0) {
                in->text[0] = '\0';
            }
            in->len = 0;
            in->cursor_pos = 0;
            input_clear_selection_internal(in);

            in->focused = 0;
            SDL_StopTextInput();
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

        /* render submitted label (always if set) and submitted text (if present) above input */
        if (in->font_path && in->submitted_label) {
            int label_size = in->font_size > 12 ? in->font_size - 4 : in->font_size;
            TTF_Font* label_font = TTF_OpenFont(in->font_path, label_size);
            if (label_font) {
                SDL_Color color = {255,255,255,255};
                const char* lbl = in->submitted_label ? in->submitted_label : "";
                const char* sub = in->submitted_text ? in->submitted_text : "";
                size_t L = strlen(lbl);
                size_t M = strlen(sub);
                char* combined = (char*)malloc(L + M + 1);
                if (combined) {
                    memcpy(combined, lbl, L);
                    memcpy(combined + L, sub, M + 1);
                    SDL_Surface* surf_label = TTF_RenderUTF8_Blended(label_font, combined, color);
                    if (surf_label) {
                        SDL_Texture* tex_label = SDL_CreateTextureFromSurface(renderer, surf_label);
                        if (tex_label) {
                            int tw = 0, th = 0;
                            SDL_QueryTexture(tex_label, NULL, NULL, &tw, &th);
                            int dst_x = in->rect.x + in->padding;
                            int dst_y = in->rect.y - th - 8;
                            SDL_Rect dst = { dst_x, dst_y, tw, th };
                            SDL_RenderCopy(renderer, tex_label, NULL, &dst);
                            SDL_DestroyTexture(tex_label);
                        }
                        SDL_FreeSurface(surf_label);
                    }
                    free(combined);
                }
                TTF_CloseFont(label_font);
            }
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
                TTF_SizeUTF8(font, before_cursor, &cx, &cy);
            int padding = in->padding;

            /* draw selection highlight if any */
            if (input_has_selection(in)) {
                int s = in->sel_start;
                int l = in->sel_len;
                if (s < 0) s = 0;
                if (s > in->len) s = in->len;
                if (l < 0) l = 0;
                if (s + l > in->len) l = in->len - s;
                if (l > 0) {
                    char* before_sel = (char*)malloc(s + 1);
                    char* sel_text = (char*)malloc(l + 1);
                    if (before_sel && sel_text) {
                        if (s > 0) memcpy(before_sel, in->text, s);
                        before_sel[s] = '\0';
                        memcpy(sel_text, in->text + s, l);
                        sel_text[l] = '\0';
                        int bx = 0, by = 0, sw = 0, sh = 0;
                            TTF_SizeUTF8(font, before_sel, &bx, &by);
                            TTF_SizeUTF8(font, sel_text, &sw, &sh);
                        SDL_Rect selrect = { in->rect.x + padding + bx, in->rect.y + (in->rect.h - sh) / 2, sw, sh };
                        if (selrect.x < in->rect.x + padding) selrect.x = in->rect.x + padding;
                        if (selrect.x + selrect.w > in->rect.x + in->rect.w - padding) selrect.w = (in->rect.x + in->rect.w - padding) - selrect.x;
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, 51, 153, 255, 128);
                        SDL_RenderFillRect(renderer, &selrect);
                    }
                    free(before_sel);
                    free(sel_text);
                }
            }

            SDL_Surface* surf = NULL;
            SDL_Texture* tex = NULL;
            /* If no text and placeholders exist, render current placeholder in gray */
            if (in->len == 0 && in->placeholders && in->placeholder_count > 0) {
                Uint32 now = SDL_GetTicks();
                if (now - in->placeholder_last_tick >= 3000) { // switch placeholder every 3 seconds
                    in->placeholder_last_tick = now;
                    in->placeholder_index = (in->placeholder_index + 1) % in->placeholder_count;
                }
                const char* ph = in->placeholders[in->placeholder_index];
                SDL_Color ph_color = {180, 180, 180, 255};
                    surf = TTF_RenderUTF8_Blended(font, ph, ph_color);
            } else {
                    surf = TTF_RenderUTF8_Blended(font, in->text, in->text_color);
            }
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
