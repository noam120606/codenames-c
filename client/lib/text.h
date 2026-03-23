#ifndef TEXT_H
#define TEXT_H

#include "sdl.h"


typedef struct TextConfig {
    const char* font_path;
    int font_size;
    SDL_Color color;
    int x;
    int y;
    double angle;
    Uint8 opacity;
} TextConfig;

typedef struct Text {
    char* content;
    TextConfig cfg;
    SDL_Texture* texture;
} Text;

TTF_Font* load_font(const char* font_path, int size);
TextConfig create_text_config(const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity);
void reload_text(AppContext* context, Text* text);
Text* create_text(const char* content, TextConfig cfg);
Text* init_text(AppContext* context, const char* content, TextConfig cfg);
void update_text(AppContext* context, Text* text, const char* new_content);
void update_text_position(Text* text, int x, int y);
void update_text_color(AppContext* context, Text* text, SDL_Color color);
void display_text(AppContext* context, Text* text);
int destroy_text(Text* text);

#endif /* TEXT_H */