#ifndef INPUT_H
#define INPUT_H

#include "sdl.h"

#define INPUT_DEFAULT_MAX 256

typedef struct Input {
    SDL_Rect rect;
    char* text;
    int maxlen;
    int len;
    int cursor_pos;
    int focused;
    int submitted;
    SDL_Color bg_color;
    SDL_Color border_color;
    SDL_Color text_color;
    const char* font_path;
    int font_size;
    void (*on_submit)(const char*);
} Input;

Input* input_create(int x, int y, int w, int h, const char* font_path, int font_size, int maxlen);
void input_destroy(Input* in);
void input_handle_event(Input* in, SDL_Event* e);
void input_render(SDL_Renderer* renderer, Input* in);
const char* input_get_text(Input* in);
int input_is_submitted(Input* in);
void input_clear_submitted(Input* in);
void input_set_text(Input* in, const char* text);
void input_set_on_submit(Input* in, void (*cb)(const char*));

#endif /* INPUT_H */
