#ifndef TEXT_H
#define TEXT_H

#include "sdl.h"

/**
 * Configuration structure for text rendering.
 * @param font_path Path to the TTF font file.
 * @param font_size Size of the font.
 * @param color Color of the text.
 * @param x X position of the text.
 * @param y Y position of the text.
 * @param angle Rotation angle of the text.
 * @param opacity Opacity of the text.
 */
typedef struct TextConfig {
    const char* font_path;
    int font_size;
    SDL_Color color;
    int x;
    int y;
    double angle;
    Uint8 opacity;
} TextConfig;

/**
 * Structure for rendering text.
 * @param content The string content of the text.
 * @param cfg Configuration settings for the text rendering.
 * @param texture The SDL_Texture used for rendering the text.
 */
typedef struct Text {
    char* content;
    TextConfig cfg;
    SDL_Texture* texture;
} Text;

/**
 * Load a TTF font.
 * @param font_path Path to the TTF font file.
 * @param size Size of the font.
 * @return Pointer to the loaded TTF_Font, or NULL on failure.
 */
TTF_Font* load_font(const char* font_path, int size);

/**
 * Create a text configuration.
 * @param font_path Path to the TTF font file.
 * @param size Size of the font.
 * @param color Color of the text.
 * @param x X position of the text.
 * @param y Y position of the text.
 * @param angle Rotation angle of the text.
 * @param opacity Opacity of the text.
 * @return A TextConfig structure initialized with the provided values.
 */
TextConfig create_text_config(const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity);

/**
 * Reload a text object.
 * @param context The application context.
 * @param text The text object to reload.
 */
void reload_text(AppContext* context, Text* text);

/**
 * Create a text object.
 * @param content The string content of the text.
 * @param cfg Configuration settings for the text rendering.
 * @return A pointer to the created Text object, or NULL on failure.
 */
Text* create_text(const char* content, TextConfig cfg);

/**
 * Initialize a text object.
 * @param context The application context.
 * @param content The string content of the text.
 * @param cfg Configuration settings for the text rendering.
 * @return A pointer to the initialized Text object, or NULL on failure.
 */
Text* init_text(AppContext* context, const char* content, TextConfig cfg);

/**
 * Update the content of a text object.
 * @param context The application context.
 * @param text The text object to update.
 * @param new_content The new content for the text object.
 */
void update_text(AppContext* context, Text* text, const char* new_content);

/**
 * Update the position of a text object.
 * @param text The text object to update.
 * @param x The new X position of the text.
 * @param y The new Y position of the text.
 */
void update_text_position(Text* text, int x, int y);

/**
 * Update the color of a text object.
 * @param context The application context.
 * @param text The text object to update.
 * @param color The new color for the text object.
 */
void update_text_color(AppContext* context, Text* text, SDL_Color color);

/**
 * Display a text object.
 * @param context The application context.
 * @param text The text object to display.
 */
void display_text(AppContext* context, Text* text);

/**
 * Destroy a text object.
 * @param text The text object to destroy.
 * @return 0 on success, -1 on failure.
 */
int destroy_text(Text* text);

#endif /* TEXT_H */