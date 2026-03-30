/* Input widget implementation */
#include "../lib/all.h"

#define MAX_INPUTS 32
static Input* inputs[MAX_INPUTS];
static int input_count = 0;

static Input* input_find_by_id(InputId id) {
    for (int i = 0; i < input_count; ++i) {
        if (inputs[i] && inputs[i]->id == id) return inputs[i];
    }
    return NULL;
}

static void input_clear_selection_internal(Input* in);

static int utf8_is_continuation_byte(unsigned char c) {
    return (c & 0xC0) == 0x80;
}

static int utf8_prev_char_pos(const char* text, int pos) {
    if (!text || pos <= 0) return 0;
    int i = pos - 1;
    while (i > 0 && utf8_is_continuation_byte((unsigned char)text[i])) i--;
    return i;
}

static int utf8_next_char_pos(const char* text, int len, int pos) {
    if (!text || pos >= len) return len;
    int i = pos + 1;
    while (i < len && utf8_is_continuation_byte((unsigned char)text[i])) i++;
    return i;
}

static int regex_match_text(const char* pattern, const char* text) {
    if (!pattern || !text) return 1;

    CnRegex re;
    if (cn_regex_compile(&re, pattern) != 0) {
        /* regex invalide -> conserver le comportement permissif historique */
        return 1;
    }

    int ok = (cn_regex_match(&re, text) == 0) ? 1 : 0;
    cn_regex_free(&re);
    return ok;
}

static void input_reload_submit_sound(Input* in) {
    if (!in || !in->cfg) return;

    if (in->cfg->submit_sound_chunk) {
        Mix_FreeChunk(in->cfg->submit_sound_chunk);
        in->cfg->submit_sound_chunk = NULL;
    }

    if (!in->cfg->submit_sound || in->cfg->submit_sound[0] == '\0') return;

    in->cfg->submit_sound_chunk = Mix_LoadWAV(in->cfg->submit_sound);
    if (!in->cfg->submit_sound_chunk) {
        printf("input submit sound load failed (%s): %s\n", in->cfg->submit_sound, Mix_GetError());
    }
}

static void input_play_submit_sound(Input* in) {
    if (!in || !in->cfg || !in->cfg->submit_sound_chunk) return;

    if (Mix_PlayChannel(-1, in->cfg->submit_sound_chunk, 0) < 0) {
        printf("input submit sound play failed (%s): %s\n", in->cfg->submit_sound ? in->cfg->submit_sound : "", Mix_GetError());
    }
}

static void input_submit_internal(AppContext* context, Input* in) {
    if (!in || !in->cfg) return;

    // Requis pour les inputs du mot indice et le nombre de mots.
    if (in->cfg->disabled) return;

    if (in->cfg->submit_pattern && in->cfg->text) {
        if (!regex_match_text(in->cfg->submit_pattern, in->cfg->text)) return;
    }

    in->cfg->submitted = 1;

    if (in->cfg->submitted_text) {
        free(in->cfg->submitted_text);
        in->cfg->submitted_text = NULL;
    }

    in->cfg->submitted_text = (char*)malloc((size_t)in->cfg->len + 1);
    if (in->cfg->submitted_text) {
        strncpy(in->cfg->submitted_text, in->cfg->text, (size_t)in->cfg->len + 1);
    }

    const char* submitted = in->cfg->submitted_text ? in->cfg->submitted_text : in->cfg->text;

    input_play_submit_sound(in);

    if (in->cfg->on_submit) in->cfg->on_submit(context, submitted);

    if (in->cfg->clear_on_submit) {
        if (in->cfg->text && in->cfg->maxlen > 0) {
            in->cfg->text[0] = '\0';
        }
        in->cfg->len = 0;
        in->cfg->cursor_pos = 0;
    }

    input_clear_selection_internal(in);

    if (in->cfg->keep_focus_on_submit) {
        for (int i = 0; i < input_count; ++i) {
            if (!inputs[i] || !inputs[i]->cfg) continue;
            inputs[i]->cfg->focused = (inputs[i] == in) ? 1 : 0;
        }
    } else {
        in->cfg->focused = 0;
    }

    int has_focused_input = 0;
    for (int i = 0; i < input_count; ++i) {
        if (inputs[i] && inputs[i]->cfg && inputs[i]->cfg->focused) {
            has_focused_input = 1;
            break;
        }
    }

    if (has_focused_input) SDL_StartTextInput();
    else SDL_StopTextInput();
}

/** Initialise une InputConfig avec des valeurs par défaut. */
InputConfig* input_config_init() {
    InputConfig* cfg = (InputConfig*)calloc(1, sizeof(InputConfig));
    if (!cfg) return NULL;

    cfg->x = 0;
    cfg->y = 0;
    cfg->w = 0;
    cfg->h = 0;
    cfg->font_path = NULL;
    cfg->font_size = 16;
    cfg->placeholders = NULL;
    cfg->placeholder_count = 0;
    cfg->submitted_label = NULL;
    cfg->maxlen = INPUT_DEFAULT_MAX;
    cfg->save_player_data = 0;
    cfg->bg_color = (SDL_Color){255, 255, 255, 255};
    cfg->border_color = (SDL_Color){0, 0, 0, 255};
    cfg->text_color = (SDL_Color){255, 255, 255, 255};
    cfg->padding = 8;
    cfg->centered = 0;
    cfg->bg_path = NULL;
    cfg->bg_padding = -1;
    cfg->allowed_pattern = NULL;
    cfg->submit_pattern = NULL;
    cfg->submit_sound = NULL;
    cfg->init_text = NULL;
    cfg->on_submit = NULL;

    cfg->rect = (SDL_Rect){0, 0, 0, 0};
    cfg->text = NULL;
    cfg->disabled = 0;
    cfg->clear_on_submit = 1;
    cfg->keep_focus_on_submit = 0;
    cfg->len = 0;
    cfg->cursor_pos = 0;
    cfg->focused = 0;
    cfg->submitted = 0;
    cfg->bg_texture = NULL;
    cfg->sel_start = 0;
    cfg->sel_len = 0;
    cfg->sel_anchor = 0;
    cfg->submitted_text = NULL;
    cfg->submit_sound_chunk = NULL;
    cfg->placeholder_index = 0;
    cfg->placeholder_last_tick = 0;

    return cfg;
}

static void input_clear_selection_internal(Input* in) {
    if (!in) return;
    in->cfg->sel_start = 0;
    in->cfg->sel_len = 0;
    in->cfg->sel_anchor = 0;
}

static int input_has_selection(Input* in) {
    return in && in->cfg->sel_len > 0;
}

static void input_delete_selection(Input* in) {
    if (!in || in->cfg->sel_len <= 0) return;
    int s = in->cfg->sel_start;
    int l = in->cfg->sel_len;
    memmove(in->cfg->text + s, in->cfg->text + s + l, in->cfg->len - (s + l) + 1);
    in->cfg->len -= l;
    if (in->cfg->len < 0) in->cfg->len = 0;
    in->cfg->text[in->cfg->len] = '\0';
    in->cfg->cursor_pos = s;
    input_clear_selection_internal(in);
}

static int prev_word_pos(Input* in, int pos) {
    if (!in) return 0;
    int i = pos;
    while (i > 0 && isspace((unsigned char)in->cfg->text[i-1])) i--;
    while (i > 0 && !isspace((unsigned char)in->cfg->text[i-1])) i--;
    return i;
}

static int next_word_pos(Input* in, int pos) {
    if (!in) return 0;
    int i = pos;
    while (i < in->cfg->len && !isspace((unsigned char)in->cfg->text[i])) i++;
    while (i < in->cfg->len && isspace((unsigned char)in->cfg->text[i])) i++;
    return i;
}

Input* input_create(SDL_Renderer* renderer, InputId id, const InputConfig* cfg_in) {
    if (input_count >= MAX_INPUTS) return NULL;

    Input* in = (Input*)malloc(sizeof(Input));
    if (!in) return NULL;
    in->id = id;

    /* Input owns its config. We copy cfg_in (or defaults) into a freshly allocated struct,
     * then allocate runtime buffers (text) and reset pointers (textures). */
    in->cfg = input_config_init();
    if (!in->cfg) {
        free(in);
        return NULL;
    }
    if (cfg_in) {
        /* Shallow copy first, then fix ownership-sensitive fields below. */
        *in->cfg = *cfg_in;
    }

    /* runtime-managed fields: always start from a clean state */
    in->cfg->bg_texture = NULL;
    in->cfg->submitted_text = NULL;
    in->cfg->text = NULL;
    in->cfg->len = 0;
    in->cfg->cursor_pos = 0;
    in->cfg->focused = 0;
    in->cfg->submitted = 0;
    in->cfg->sel_start = 0;
    in->cfg->sel_len = 0;
    in->cfg->sel_anchor = 0;
    in->cfg->placeholder_index = 0;
    in->cfg->placeholder_last_tick = SDL_GetTicks();
    in->cfg->submit_sound_chunk = NULL;

    /* Conversion coordonnées relatives au centre -> écran.
     * x positif = droite, y positif = haut.
     * (x, y) représente le centre de l'input relatif au centre de la fenêtre. */
    int rel_x = in->cfg->x;
    int rel_y = in->cfg->y;
    int screen_x = (WIN_WIDTH / 2) + rel_x - (in->cfg->w / 2);
    int screen_y = (WIN_HEIGHT / 2) - rel_y - (in->cfg->h / 2);

    in->cfg->x = screen_x;
    in->cfg->y = screen_y;

    /* keep rect in sync */
    in->cfg->rect = (SDL_Rect){in->cfg->x, in->cfg->y, in->cfg->w, in->cfg->h};

    /* allocate text buffer */
    if (in->cfg->maxlen <= 0) in->cfg->maxlen = INPUT_DEFAULT_MAX;
    in->cfg->text = (char*)calloc((size_t)in->cfg->maxlen + 1, 1);
    if (!in->cfg->text) {
        input_destroy(in);
        return NULL;
    }

    /* placeholders: just reference caller-provided strings (do not strdup),
     * so stack/const arrays are safe as long as they outlive the input.
     * (This matches current usage in menu.c.) */
    /* submitted_label is a const char*; no allocation here */

    if (in->cfg->init_text && in->cfg->init_text[0] != '\0') {
        strncpy(in->cfg->text, in->cfg->init_text, (size_t)in->cfg->maxlen);
        in->cfg->text[in->cfg->maxlen] = '\0';
        in->cfg->len = (int)strlen(in->cfg->text);
        in->cfg->cursor_pos = in->cfg->len;
    }

    /* Load saved player name from properties file if persistence is enabled */
    if (in->cfg->save_player_data && in->id == INPUT_NAME) {
        char saved_name[INPUT_DEFAULT_MAX + 1] = {0};
        if (read_property(saved_name, "PLAYER_NAME") == EXIT_SUCCESS && saved_name[0] != '\0') {
            strncpy(in->cfg->text, saved_name, (size_t)in->cfg->maxlen);
            in->cfg->text[in->cfg->maxlen] = '\0';
            in->cfg->len = (int)strlen(in->cfg->text);
            in->cfg->cursor_pos = in->cfg->len;
        }
    }

    /* auto-load background image if bg_path was provided in config */
    if (in->cfg->bg_path && renderer) {
        input_set_bg(in, renderer, in->cfg->bg_path, in->cfg->bg_padding);
    }

    /* preload submit sound if configured */
    input_reload_submit_sound(in);

    inputs[input_count++] = in;

    return in;
}

void input_destroy(Input* in) {
    if (!in) return;

    for (int i = 0; i < input_count; ++i) {
        if (inputs[i] == in) {
            for (int j = i; j < input_count - 1; ++j) inputs[j] = inputs[j + 1];
            inputs[input_count - 1] = NULL;
            input_count--;
            break;
        }
    }

    if (in->cfg) {
        if (in->cfg->text) free(in->cfg->text);
        if (in->cfg->bg_texture) free_image(in->cfg->bg_texture);
        if (in->cfg->submitted_text) free(in->cfg->submitted_text);
        if (in->cfg->submit_sound_chunk) Mix_FreeChunk(in->cfg->submit_sound_chunk);
        /* submitted_label and placeholders are not owned (const pointers) */
        free(in->cfg);
    }
    free(in);
}

/** Vérifie si le texte `txt` correspond à la regex `allowed_pattern`.
 *  Retourne 1 si le texte est accepté, 0 sinon. */
static int input_text_allowed(const char* allowed_pattern, const char* txt) {
    if (!allowed_pattern || !txt) return 1;
    int len = (int)strlen(txt);
    for (int i = 0; i < len; i++) {
        char ch[2] = { txt[i], '\0' };
        if (!regex_match_text(allowed_pattern, ch)) {
            return 0;
        }
    }
    return 1;
}

static int point_in_rect(int x, int y, SDL_Rect* r) {
    return x >= r->x && x <= (r->x + r->w) && y >= r->y && y <= (r->y + r->h);
}

void input_handle_event(AppContext* context, Input* in, SDL_Event* e) {
    if (!in || !e) return;

    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int mx = e->button.x;
        int my = e->button.y;
        if (point_in_rect(mx, my, &in->cfg->rect)) {
            in->cfg->focused = 1;
            SDL_StartTextInput();
            /* place cursor at end */
            in->cfg->cursor_pos = in->cfg->len;
        } else {
            if (in->cfg->focused) {
                in->cfg->focused = 0;
                SDL_StopTextInput();
            }
        }
    }

    if (!in->cfg->focused) return;

    if (e->type == SDL_TEXTINPUT) {
        const char* txt = e->text.text;
        int add = strlen(txt);
        if (in->cfg->len + add > in->cfg->maxlen) return;
        /* filter characters against allowed_pattern if set */
        if (!input_text_allowed(in->cfg->allowed_pattern, txt)) return;
        /* if there is a selection, replace it */
        if (input_has_selection(in)) {
            input_delete_selection(in);
        }
        /* insert at cursor_pos */
        memmove(in->cfg->text + in->cfg->cursor_pos + add, in->cfg->text + in->cfg->cursor_pos, in->cfg->len - in->cfg->cursor_pos + 1);
        memcpy(in->cfg->text + in->cfg->cursor_pos, txt, add);
        in->cfg->cursor_pos += add;
        in->cfg->len += add;
    }

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        SDL_Keymod mod = e->key.keysym.mod;
        /* Ctrl+A = select all */
        if ((mod & KMOD_CTRL) && (k == SDLK_a)) {
            in->cfg->sel_start = 0;
            in->cfg->sel_len = in->cfg->len;
            in->cfg->sel_anchor = 0;
            in->cfg->cursor_pos = in->cfg->len;
            return;
        }

        if (k == SDLK_BACKSPACE) {
            if (input_has_selection(in)) {
                input_delete_selection(in);
            } else if (mod & KMOD_CTRL) {
                int np = prev_word_pos(in, in->cfg->cursor_pos);
                int del = in->cfg->cursor_pos - np;
                if (del > 0) {
                    memmove(in->cfg->text + np, in->cfg->text + in->cfg->cursor_pos, in->cfg->len - in->cfg->cursor_pos + 1);
                    in->cfg->len -= del;
                    in->cfg->cursor_pos = np;
                    in->cfg->text[in->cfg->len] = '\0';
                }
            } else {
                if (in->cfg->cursor_pos > 0 && in->cfg->len > 0) {
                    int np = utf8_prev_char_pos(in->cfg->text, in->cfg->cursor_pos);
                    int del = in->cfg->cursor_pos - np;
                    memmove(in->cfg->text + np, in->cfg->text + in->cfg->cursor_pos, in->cfg->len - in->cfg->cursor_pos + 1);
                    in->cfg->cursor_pos = np;
                    in->cfg->len -= del;
                    in->cfg->text[in->cfg->len] = '\0';
                }
            }
            input_clear_selection_internal(in);
        } else if (k == SDLK_DELETE) {
            if (input_has_selection(in)) {
                input_delete_selection(in);
            } else if (mod & KMOD_CTRL) {
                int np = next_word_pos(in, in->cfg->cursor_pos);
                int del = np - in->cfg->cursor_pos;
                if (del > 0) {
                    memmove(in->cfg->text + in->cfg->cursor_pos, in->cfg->text + np, in->cfg->len - np + 1);
                    in->cfg->len -= del;
                    in->cfg->text[in->cfg->len] = '\0';
                }
            } else {
                if (in->cfg->cursor_pos < in->cfg->len && in->cfg->len > 0) {
                    int np = utf8_next_char_pos(in->cfg->text, in->cfg->len, in->cfg->cursor_pos);
                    int del = np - in->cfg->cursor_pos;
                    memmove(in->cfg->text + in->cfg->cursor_pos, in->cfg->text + np, in->cfg->len - np + 1);
                    in->cfg->len -= del;
                    in->cfg->text[in->cfg->len] = '\0';
                }
            }
            input_clear_selection_internal(in);
        } else if (k == SDLK_LEFT) {
            int newpos = in->cfg->cursor_pos;
            if (mod & KMOD_CTRL) newpos = prev_word_pos(in, in->cfg->cursor_pos);
            else if (in->cfg->cursor_pos > 0) newpos = utf8_prev_char_pos(in->cfg->text, in->cfg->cursor_pos);

            if (mod & KMOD_SHIFT) {
                if (!input_has_selection(in)) {
                    in->cfg->sel_anchor = in->cfg->cursor_pos;
                    in->cfg->sel_start = in->cfg->cursor_pos;
                }
                in->cfg->cursor_pos = newpos;
                int s = in->cfg->sel_anchor;
                int epos = in->cfg->cursor_pos;
                if (epos < s) { in->cfg->sel_start = epos; in->cfg->sel_len = s - epos; }
                else { in->cfg->sel_start = s; in->cfg->sel_len = epos - s; }
            } else {
                in->cfg->cursor_pos = newpos;
                input_clear_selection_internal(in);
            }
        } else if (k == SDLK_RIGHT) {
            int newpos = in->cfg->cursor_pos;
            if (mod & KMOD_CTRL) newpos = next_word_pos(in, in->cfg->cursor_pos);
            else if (in->cfg->cursor_pos < in->cfg->len) newpos = utf8_next_char_pos(in->cfg->text, in->cfg->len, in->cfg->cursor_pos);

            if (mod & KMOD_SHIFT) {
                if (!input_has_selection(in)) {
                    in->cfg->sel_anchor = in->cfg->cursor_pos;
                    in->cfg->sel_start = in->cfg->cursor_pos;
                }
                in->cfg->cursor_pos = newpos;
                int s = in->cfg->sel_anchor;
                int epos = in->cfg->cursor_pos;
                if (epos < s) { in->cfg->sel_start = epos; in->cfg->sel_len = s - epos; }
                else { in->cfg->sel_start = s; in->cfg->sel_len = epos - s; }
            } else {
                in->cfg->cursor_pos = newpos;
                input_clear_selection_internal(in);
            }
        } else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            input_submit_internal(context, in);
        }
    }
}

void input_render(SDL_Renderer* renderer, Input* in) {
    if (!renderer || !in) return;
    SDL_Rect rect = (SDL_Rect){ in->cfg->x, in->cfg->y, in->cfg->w, in->cfg->h };
    in->cfg->rect = rect;
    
    /* background: image if present else color */
    if (in->cfg->bg_texture) {
        SDL_RenderCopy(renderer, in->cfg->bg_texture, NULL, &rect);
    } else {
        SDL_SetRenderDrawColor(renderer, in->cfg->bg_color.r, in->cfg->bg_color.g, in->cfg->bg_color.b, in->cfg->bg_color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    /* border: draw only when no background texture is set */
    if (!in->cfg->bg_texture) {
        SDL_SetRenderDrawColor(renderer, in->cfg->border_color.r, in->cfg->border_color.g, in->cfg->border_color.b, in->cfg->border_color.a);
        SDL_RenderDrawRect(renderer, &rect);
    }

        /* render submitted label (always if set) and submitted text (if present) above input */
        if (in->cfg->font_path && in->cfg->submitted_label) {
            int label_size = in->cfg->font_size > 12 ? in->cfg->font_size - 4 : in->cfg->font_size;
            TTF_Font* label_font = TTF_OpenFont(in->cfg->font_path, label_size);
            if (label_font) {
                SDL_Color color = {255,255,255,255};
                const char* lbl = in->cfg->submitted_label ? in->cfg->submitted_label : "";
                const char* sub = in->cfg->submitted_text ? in->cfg->submitted_text : "";
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
                            int dst_x = in->cfg->x + in->cfg->padding;
                            int dst_y = in->cfg->y - th - 8;
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
    if (in->cfg->font_path && in->cfg->font_size > 0) {
        TTF_Font* font = TTF_OpenFont(in->cfg->font_path, in->cfg->font_size);
        if (font) {
            /* compute width of text up to cursor so we can draw cursor even when no
             * texture is created (empty string case) */
            char before_cursor[512];
            int n = in->cfg->cursor_pos;
            if (n > 0) {
                if (n >= (int)sizeof(before_cursor)) n = sizeof(before_cursor)-1;
                memcpy(before_cursor, in->cfg->text, n);
                before_cursor[n] = '\0';
            } else before_cursor[0] = '\0';

            int cx = 0, cy = 0;
                TTF_SizeUTF8(font, before_cursor, &cx, &cy);
            int padding = in->cfg->padding;

            /* compute full text width to determine centering offset */
            int full_tw = 0, full_th = 0;
            TTF_SizeUTF8(font, in->cfg->text, &full_tw, &full_th);
            int text_offset_x = padding; /* default: left-aligned with padding */
            if (in->cfg->centered) {
                int center_off = (in->cfg->w - full_tw) / 2;
                if (center_off < padding) center_off = padding;
                text_offset_x = center_off;
            }

            /* draw selection highlight if any */
            if (input_has_selection(in)) {
                int s = in->cfg->sel_start;
                int l = in->cfg->sel_len;
                if (s < 0) s = 0;
                if (s > in->cfg->len) s = in->cfg->len;
                if (l < 0) l = 0;
                if (s + l > in->cfg->len) l = in->cfg->len - s;
                if (l > 0) {
                    char* before_sel = (char*)malloc(s + 1);
                    char* sel_text = (char*)malloc(l + 1);
                    if (before_sel && sel_text) {
                        if (s > 0) memcpy(before_sel, in->cfg->text, s);
                        before_sel[s] = '\0';
                        memcpy(sel_text, in->cfg->text + s, l);
                        sel_text[l] = '\0';
                        int bx = 0, by = 0, sw = 0, sh = 0;
                            TTF_SizeUTF8(font, before_sel, &bx, &by);
                            TTF_SizeUTF8(font, sel_text, &sw, &sh);
                        SDL_Rect selrect = { in->cfg->x + text_offset_x + bx, in->cfg->y + (in->cfg->h - sh) / 2, sw, sh };
                        if (selrect.x < in->cfg->x + padding) selrect.x = in->cfg->x + padding;
                        if (selrect.x + selrect.w > in->cfg->x + in->cfg->w - padding) selrect.w = (in->cfg->x + in->cfg->w - padding) - selrect.x;
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
            if (in->cfg->len == 0 && in->cfg->placeholders && in->cfg->placeholder_count > 0) {
                Uint32 now = SDL_GetTicks();
                if (now - in->cfg->placeholder_last_tick >= 3000) { // switch placeholder every 3 seconds
                    in->cfg->placeholder_last_tick = now;
                    in->cfg->placeholder_index = (in->cfg->placeholder_index + 1) % in->cfg->placeholder_count;
                }
                const char* ph = in->cfg->placeholders[in->cfg->placeholder_index];
                SDL_Color ph_color = {180, 180, 180, 255};
                    surf = TTF_RenderUTF8_Blended(font, ph, ph_color);
            } else {
                    surf = TTF_RenderUTF8_Blended(font, in->cfg->text, in->cfg->text_color);
            }
            if (surf) {
                tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int tw = 0, th = 0;
                    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
                    /* For placeholder centering, recompute offset based on placeholder width */
                    int dst_x = in->cfg->x + text_offset_x;
                    if (in->cfg->centered && in->cfg->len == 0 && in->cfg->placeholders && in->cfg->placeholder_count > 0) {
                        int center_off = (in->cfg->w - tw) / 2;
                        if (center_off < padding) center_off = padding;
                        dst_x = in->cfg->x + center_off;
                    }
                    SDL_Rect dst = { dst_x, in->cfg->y + (in->cfg->h - th) / 2, tw, th };

                    /* If text wider than input, clip source width */
                    SDL_Rect src = {0, 0, tw, th};
                    if (dst.w > in->cfg->w - padding*2) {
                        src.w = (in->cfg->w - padding*2) * ((float)tw / dst.w);
                        dst.w = in->cfg->w - padding*2;
                    }

                    SDL_RenderCopy(renderer, tex, &src, &dst);
                }
            }

            /* draw cursor if focused (draw regardless of surf/tex) */
            if (in->cfg->focused) {
                int cursor_x = in->cfg->x + text_offset_x + cx;
                /* ensure cursor stays inside rect */
                if (cursor_x > in->cfg->x + in->cfg->w - padding) cursor_x = in->cfg->x + in->cfg->w - padding;

                /* blink */
                Uint32 t = SDL_GetTicks();
                if ((t / 500) % 2 == 0) {
                    SDL_SetRenderDrawColor(renderer, in->cfg->text_color.r, in->cfg->text_color.g, in->cfg->text_color.b, in->cfg->text_color.a);
                    SDL_Rect cur = { cursor_x, in->cfg->y + 6, 2, in->cfg->h - 12 };
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
    return in->cfg->text;
}

int input_is_submitted(Input* in) {
    if (!in) return 0;
    return in->cfg->submitted;
}

void input_clear_submitted(Input* in) {
    if (!in) return;
    in->cfg->submitted = 0;
}

void input_submit(AppContext* context, Input* in) {
    input_submit_internal(context, in);
}

void input_set_text(Input* in, const char* text) {
    if (!in || !text) return;
    strncpy(in->cfg->text, text, in->cfg->maxlen);
    in->cfg->text[in->cfg->maxlen] = '\0';
    in->cfg->len = strlen(in->cfg->text);
    in->cfg->cursor_pos = in->cfg->len;
    in->cfg->sel_anchor = 0;
    input_clear_selection_internal(in);
}

void input_set_on_submit(Input* in, void (*cb)(AppContext*, const char*)) {
    if (!in) return;
    in->cfg->on_submit = cb;
}

int input_set_bg(Input* in, SDL_Renderer* renderer, const char* path, int padding) {
    if (!in || !renderer || !path) return EXIT_FAILURE;
    if (in->cfg->bg_texture) {
        free_image(in->cfg->bg_texture);
        in->cfg->bg_texture = NULL;
    }
    SDL_Texture* tex = load_image(renderer, path);
    if (!tex) return EXIT_FAILURE;
    /* If caller provided padding >= 0, use it; otherwise adapt to texture */
    if (padding >= 0) {
        in->cfg->padding = padding;
    } else {
        int tw = 0, th = 0;
        if (SDL_QueryTexture(tex, NULL, NULL, &tw, &th) == 0) {
            int p = in->cfg->h / 3;
            if (p < 6) p = 6;
            if (tw > 0 && tw < in->cfg->w) p += 4;
            in->cfg->padding = p;
        }
    }
    in->cfg->bg_texture = tex;
    return EXIT_SUCCESS;
}

void input_clear_bg(Input* in) {
    if (!in) return;
    if (in->cfg->bg_texture) {
        free_image(in->cfg->bg_texture);
        in->cfg->bg_texture = NULL;
    }
    in->cfg->padding = 8;
}

void input_set_padding(Input* in, int padding) {
    if (!in) return;
    if (padding < 0) padding = 0;
    in->cfg->padding = padding;
}

int edit_in_cfg(InputId id, InputCfgKey key, intptr_t value) {
    Input* in = input_find_by_id(id);
    if (!in || !in->cfg) return EXIT_FAILURE;

    switch (key) {
        case IN_CFG_X:
            in->cfg->x = (int)value;
            in->cfg->rect.x = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_Y:
            in->cfg->y = (int)value;
            in->cfg->rect.y = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_W:
            in->cfg->w = (int)value;
            in->cfg->rect.w = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_H:
            in->cfg->h = (int)value;
            in->cfg->rect.h = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_FONT_PATH:
            in->cfg->font_path = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_FONT_SIZE:
            in->cfg->font_size = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_PLACEHOLDERS:
            in->cfg->placeholders = (const char**)value;
            return EXIT_SUCCESS;
        case IN_CFG_PLACEHOLDER_COUNT:
            in->cfg->placeholder_count = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_SUBMITTED_LABEL:
            in->cfg->submitted_label = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_MAXLEN: {
            int new_maxlen = (int)value;
            if (new_maxlen <= 0) return EXIT_FAILURE;

            char* new_text = (char*)calloc((size_t)new_maxlen + 1, 1);
            if (!new_text) return EXIT_FAILURE;

            if (in->cfg->text) {
                size_t old_len = strlen(in->cfg->text);
                size_t copy_len = old_len;
                if (copy_len > (size_t)new_maxlen) copy_len = (size_t)new_maxlen;
                memcpy(new_text, in->cfg->text, copy_len);
                new_text[copy_len] = '\0';
                free(in->cfg->text);
            }

            in->cfg->text = new_text;
            in->cfg->maxlen = new_maxlen;
            in->cfg->len = (int)strlen(in->cfg->text);
            if (in->cfg->cursor_pos > in->cfg->len) in->cfg->cursor_pos = in->cfg->len;
            if (in->cfg->sel_start > in->cfg->len) in->cfg->sel_start = in->cfg->len;
            if (in->cfg->sel_start + in->cfg->sel_len > in->cfg->len) {
                in->cfg->sel_len = in->cfg->len - in->cfg->sel_start;
                if (in->cfg->sel_len < 0) in->cfg->sel_len = 0;
            }
            return EXIT_SUCCESS;
        }
        case IN_CFG_SAVE_PLAYER_DATA:
            in->cfg->save_player_data = ((int)value != 0);
            return EXIT_SUCCESS;
        case IN_CFG_BG_COLOR:
            if (!value) return EXIT_FAILURE;
            in->cfg->bg_color = *(const SDL_Color*)value;
            return EXIT_SUCCESS;
        case IN_CFG_BORDER_COLOR:
            if (!value) return EXIT_FAILURE;
            in->cfg->border_color = *(const SDL_Color*)value;
            return EXIT_SUCCESS;
        case IN_CFG_TEXT_COLOR:
            if (!value) return EXIT_FAILURE;
            in->cfg->text_color = *(const SDL_Color*)value;
            return EXIT_SUCCESS;
        case IN_CFG_PADDING:
            in->cfg->padding = (int)value;
            if (in->cfg->padding < 0) in->cfg->padding = 0;
            return EXIT_SUCCESS;
        case IN_CFG_CENTERED:
            in->cfg->centered = ((int)value != 0);
            return EXIT_SUCCESS;
        case IN_CFG_BG_PATH:
            in->cfg->bg_path = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_BG_PADDING:
            in->cfg->bg_padding = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_ALLOWED_PATTERN:
            in->cfg->allowed_pattern = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_SUBMIT_PATTERN:
            in->cfg->submit_pattern = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_SUBMIT_SOUND:
            in->cfg->submit_sound = (const char*)value;
            input_reload_submit_sound(in);
            return EXIT_SUCCESS;
        case IN_CFG_INIT_TEXT:
            in->cfg->init_text = (const char*)value;
            return EXIT_SUCCESS;
        case IN_CFG_RECT:
            if (!value) return EXIT_FAILURE;
            in->cfg->rect = *(const SDL_Rect*)value;
            in->cfg->x = in->cfg->rect.x;
            in->cfg->y = in->cfg->rect.y;
            in->cfg->w = in->cfg->rect.w;
            in->cfg->h = in->cfg->rect.h;
            return EXIT_SUCCESS;
        case IN_CFG_TEXT: {
            const char* text_value = (const char*)value;
            if (!text_value) text_value = "";
            input_set_text(in, text_value);
            return EXIT_SUCCESS;
        }
        case IN_CFG_LEN: {
            int new_len = (int)value;
            if (new_len < 0) new_len = 0;
            int current_len = in->cfg->text ? (int)strlen(in->cfg->text) : 0;
            if (new_len > current_len) new_len = current_len;
            in->cfg->len = new_len;
            if (in->cfg->text) in->cfg->text[new_len] = '\0';
            if (in->cfg->cursor_pos > in->cfg->len) in->cfg->cursor_pos = in->cfg->len;
            return EXIT_SUCCESS;
        }
        case IN_CFG_CURSOR_POS:
            in->cfg->cursor_pos = (int)value;
            if (in->cfg->cursor_pos < 0) in->cfg->cursor_pos = 0;
            if (in->cfg->cursor_pos > in->cfg->len) in->cfg->cursor_pos = in->cfg->len;
            return EXIT_SUCCESS;
        case IN_CFG_CLEAR_ON_SUBMIT:
            in->cfg->clear_on_submit = ((int)value != 0);
            return EXIT_SUCCESS;
        case IN_CFG_KEEP_FOCUS_ON_SUBMIT:
            in->cfg->keep_focus_on_submit = ((int)value != 0);
            return EXIT_SUCCESS;
        case IN_CFG_FOCUSED:
            in->cfg->focused = ((int)value != 0);
            if (in->cfg->focused) SDL_StartTextInput();
            else SDL_StopTextInput();
            return EXIT_SUCCESS;
        case IN_CFG_SUBMITTED:
            in->cfg->submitted = ((int)value != 0);
            return EXIT_SUCCESS;
        case IN_CFG_BG_TEXTURE:
            in->cfg->bg_texture = (SDL_Texture*)value;
            return EXIT_SUCCESS;
        case IN_CFG_SEL_START:
            in->cfg->sel_start = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_SEL_LEN:
            in->cfg->sel_len = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_SEL_ANCHOR:
            in->cfg->sel_anchor = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_SUBMITTED_TEXT: {
            const char* submitted_value = (const char*)value;
            if (in->cfg->submitted_text) {
                free(in->cfg->submitted_text);
                in->cfg->submitted_text = NULL;
            }
            if (submitted_value) {
                in->cfg->submitted_text = strdup(submitted_value);
                if (!in->cfg->submitted_text) return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        case IN_CFG_PLACEHOLDER_INDEX:
            in->cfg->placeholder_index = (int)value;
            return EXIT_SUCCESS;
        case IN_CFG_PLACEHOLDER_LAST_TICK:
            in->cfg->placeholder_last_tick = (Uint32)value;
            return EXIT_SUCCESS;
        case IN_CFG_ON_SUBMIT:
            if (!value) return EXIT_FAILURE;
            in->cfg->on_submit = *(void (**)(AppContext*, const char*))value;
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
    }
}
