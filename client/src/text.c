#include "../lib/all.h"

#define TEXT_FONT_CACHE_SIZE 16
#define TEXT_FONT_PATH_MAX 260

typedef struct TextFontCacheEntry {
    char path[TEXT_FONT_PATH_MAX];
    int size;
    TTF_Font* font;
} TextFontCacheEntry;

static TextFontCacheEntry text_font_cache[TEXT_FONT_CACHE_SIZE] = {0};

static TTF_Font* text_get_cached_font(const char* font_path, int size) {
    if (!font_path || font_path[0] == '\0' || size <= 0) return NULL;

    for (int i = 0; i < TEXT_FONT_CACHE_SIZE; i++) {
        TextFontCacheEntry* entry = &text_font_cache[i];
        if (!entry->font) continue;
        if (entry->size == size && strcmp(entry->path, font_path) == 0) {
            return entry->font;
        }
    }

    int slot = -1;
    for (int i = 0; i < TEXT_FONT_CACHE_SIZE; i++) {
        if (!text_font_cache[i].font) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        slot = 0;
        if (text_font_cache[slot].font) {
            TTF_CloseFont(text_font_cache[slot].font);
            text_font_cache[slot].font = NULL;
        }
    }

    TTF_Font* loaded = TTF_OpenFont(font_path, size);
    if (!loaded) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }

    TextFontCacheEntry* entry = &text_font_cache[slot];
    entry->size = size;
    strncpy(entry->path, font_path, TEXT_FONT_PATH_MAX - 1);
    entry->path[TEXT_FONT_PATH_MAX - 1] = '\0';
    entry->font = loaded;
    return entry->font;
}

TTF_Font* load_font(const char* font_path, int size) {
    TTF_Font* font = text_get_cached_font(font_path, size);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }
    return font;
}

TextConfig create_text_config(const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity) {
    TextConfig cfg;
    cfg.font_path = font_path;
    cfg.font_size = size;
    cfg.color = color;
    cfg.x = x;
    cfg.y = y;
    cfg.angle = angle;
    cfg.opacity = opacity;
    return cfg;
}

void reload_text(AppContext* context, Text* text) {
    if (!text || !context) return;

    if (text->texture) {
        SDL_DestroyTexture(text->texture);
        text->texture = NULL;
    }

    if (!text->content || text->content[0] == '\0') return;

    TTF_Font* font = text_get_cached_font(text->cfg.font_path, text->cfg.font_size);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text->content, text->cfg.color);
    if (!surface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        return;
    }

    text->texture = SDL_CreateTextureFromSurface(context->renderer, surface);
    SDL_FreeSurface(surface);

    if (!text->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        return;
    }
    
}

Text* create_text(const char* content, TextConfig cfg) {
    Text* text = (Text*)malloc(sizeof(Text));
    if (!text) {
        printf("Failed to allocate memory for Text\n");
        return NULL;
    }
    text->content = strdup(content);
    text->cfg = cfg;
    text->texture = NULL;
    return text;
}

Text* init_text(AppContext* context, const char* content, TextConfig cfg) {
    Text* text = create_text(content, cfg);
    if (text) {
        reload_text(context, text);
    }
    return text;
}

void update_text(AppContext* context, Text* text, const char* new_content) {
    if (!text || !new_content) return;
    /* Ne recharger que si le contenu a changé */
    if (text->content && strcmp(text->content, new_content) == 0) return;
    
    free(text->content);
    text->content = strdup(new_content);
    reload_text(context, text);
}

void update_text_position(Text* text, int x, int y) {
    if (!text) return;
    text->cfg.x = x;
    text->cfg.y = y;
}

void update_text_color(AppContext* context, Text* text, SDL_Color color) {
    if (!text) return;
    /* Ne recharger que si la couleur a changé */
    if (text->cfg.color.r == color.r && text->cfg.color.g == color.g && 
        text->cfg.color.b == color.b && text->cfg.color.a == color.a) return;
    
    text->cfg.color = color;
    reload_text(context, text);
}

void display_text(AppContext* context, Text* text) {
    if (!context || !text || !text->texture) return;

    int tex_w = 0, tex_h = 0;

    /* Appliquer l'opacité à la texture du texte */
    SDL_SetTextureAlphaMod(text->texture, text->cfg.opacity);

    /* Appliquer l'opacité */
    SDL_QueryTexture(text->texture, NULL, NULL, &tex_w, &tex_h);

    /* Point de pivot pour la rotation */
    SDL_Point center = { tex_w / 2, tex_h / 2 };

    // Centrer le texte dans la fenêtre
    int final_x = (WIN_WIDTH - tex_w) / 2 + text->cfg.x;
    int final_y = (WIN_HEIGHT - tex_h) / 2 - text->cfg.y; /* Inversion de l'axe y (seulement)*/

    SDL_Rect dst_rect = { final_x, final_y, tex_w, tex_h };
    SDL_RenderCopyEx(context->renderer, text->texture, NULL, &dst_rect, text->cfg.angle, &center, SDL_FLIP_NONE); // FLIP_NONE par défaut pour le moment

}

int destroy_text(Text* text) {
    if (!text) return EXIT_FAILURE;
    if (text->texture) {
        SDL_DestroyTexture(text->texture);
        text->texture = NULL;
    }
    free(text->content);
    free(text);
    return EXIT_SUCCESS;
}