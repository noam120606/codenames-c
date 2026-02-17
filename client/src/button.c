#include "../lib/all.h"

#define MAX_BUTTONS 100

static Button* buttons[MAX_BUTTONS];
static int button_count = 0;

void buttons_init() {
    for (int i = 0; i < MAX_BUTTONS; i++) {
        buttons[i] = NULL;
    }
    button_count = 0;
}

Button* button_create(int id, int x, int y, int w, int h, SDL_Texture* texture, ButtonCallback callback) {
    if (button_count >= MAX_BUTTONS) {
        printf("Erreur: nombre maximum de boutons atteint (%d)\n", MAX_BUTTONS);
        return NULL;
    }

    if (!texture) {
        printf("Erreur: texture NULL pour le bouton %d\n", id);
        return NULL;
    }

    Button* button = (Button*)malloc(sizeof(Button));
    if (!button) {
        printf("Erreur: allocation mémoire échouée pour le bouton %d\n", id);
        return NULL;
    }

    button->id = id;
    button->rect.x = x;
    button->rect.y = y;
    button->rect.w = w;
    button->rect.h = h;
    button->texture = texture;
    button->is_hovered = 0;
    button->is_text = 0;
    button->hidden = 0;
    button->callback = callback;

    buttons[button_count++] = button;

    return button;
}

Button* text_button_create(SDL_Renderer* renderer, int id, int x, int y, int taille,
                            const char* text, const char* font_path, SDL_Color color,
                            ButtonCallback callback) {

    if (!renderer || !text || !font_path) {
        printf("text_button_create: invalid argument\n");
        return NULL;
    }

    TTF_Font* font = TTF_OpenFont(font_path, 128);
    if (!font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font, text, color);
    TTF_CloseFont(font);
    if (!text_surf) {
        printf("TTF_RenderUTF8_Blended failed: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);
    int text_tex_w = text_surf->w;
    int text_tex_h = text_surf->h;
    SDL_FreeSurface(text_surf);
    if (!text_texture) {
        printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Texture* background = load_image(renderer, "assets/img/buttons/empty.png");
    if (!background) {
        printf("load_image failed: %s\n", SDL_GetError());
        SDL_DestroyTexture(text_texture);
        return NULL;
    }

    int bg_tex_w = 0, bg_tex_h = 0;
    if (SDL_QueryTexture(background, NULL, NULL, &bg_tex_w, &bg_tex_h) != 0) {
        printf("SDL_QueryTexture Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(text_texture);
        return NULL;
    }

    Button* button = (Button*)malloc(sizeof(Button));
    if (!button) {
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(text_texture);
        return NULL;
    }

    int padding = 16;

    /* Calculer la taille du bouton en conservant le ratio de l'image de fond */
    int btn_h = taille;
    int btn_w = (bg_tex_w * btn_h) / bg_tex_h;
    if (btn_w <= 0) btn_w = btn_h; /* fallback */

    /* Calculer la taille du rectangle de texte en fonction de la texture de texte
       On adapte d'abord la hauteur disponible, puis on met à l'échelle la largeur.
       Si la largeur dépasse l'espace disponible, on réduit la taille proportionnellement. */
    SDL_Rect text_rect;
    text_rect.h = btn_h - 2 * padding;
    if (text_rect.h <= 0) text_rect.h = btn_h; /* fallback */

    double scale = (text_tex_h > 0) ? ((double)text_rect.h / (double)text_tex_h) : 1.0;
    text_rect.w = (int)(text_tex_w * scale);

    int max_text_w = btn_w - 2 * padding;
    if (text_rect.w > max_text_w) {
        /* réduire pour tenir dans la largeur disponible */
        if (text_tex_w > 0) {
            double scale2 = (double)max_text_w / (double)text_tex_w;
            text_rect.w = max_text_w;
            text_rect.h = (int)(text_tex_h * scale2);
            if (text_rect.h <= 0) text_rect.h = 1;
        } else {
            text_rect.w = max_text_w;
        }
    }

    /* Centrer le texte dans le bouton */
    text_rect.x = x + (btn_w - text_rect.w) / 2;
    text_rect.y = y + (btn_h - text_rect.h) / 2;

    button->id = id;
    button->rect = (SDL_Rect){x, y, btn_w, btn_h};
    button->texture = background;
    button->is_hovered = 0;
    button->is_text = 1;
    button->text_rect = text_rect;
    button->text_texture = text_texture;
    button->hidden = 0;
    button->callback = callback;

    buttons[button_count++] = button;

    return button;
}

static int is_mouse_over_button(Button* button, int mouseX, int mouseY) {
    if (!button) {
        return 0;
    }
    return (mouseX >= button->rect.x && 
            mouseX <= button->rect.x + button->rect.w &&
            mouseY >= button->rect.y && 
            mouseY <= button->rect.y + button->rect.h);
}

ButtonReturn buttons_handle_event(SDL_Context* context, SDL_Event* event) {
    if (!event) {
        return BTN_RET_NONE;
    }

    if (event->type == SDL_MOUSEMOTION) {
        int mouseX = event->motion.x;
        int mouseY = event->motion.y;

        // Mettre à jour l'état de survol pour tous les boutons
        for (int i = 0; i < button_count; i++) {
            if (buttons[i] && !buttons[i]->hidden) {
                buttons[i]->is_hovered = is_mouse_over_button(buttons[i], mouseX, mouseY);
            } else if (buttons[i]) {
                buttons[i]->is_hovered = 0; // Ne pas surligner les boutons cachés
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mouseX = event->button.x;
        int mouseY = event->button.y;

        // Vérifier et déclencher les callbacks des boutons cliqués
        for (int i = 0; i < button_count; i++) {
            if (buttons[i] && !buttons[i]->hidden && is_mouse_over_button(buttons[i], mouseX, mouseY)) {
                if (buttons[i]->callback) {
                    return buttons[i]->callback(context, buttons[i]->id);
                }
            }
        }
    }
    // Si aucun bouton n'a été cliqué, retourner BTN_RET_NONE (avec un oubli ça avait tout cassé)
    return BTN_RET_NONE;
}

void buttons_display(SDL_Renderer* renderer) {
    if (!renderer) {
        return;
    }

    for (int i = 0; i < button_count; i++) {
        if (buttons[i] && !buttons[i]->hidden && buttons[i]->texture) {

            // Grandir le bouton au survol
            SDL_Rect render_rect = buttons[i]->rect;
            SDL_Rect text_rect = buttons[i]->text_rect;
            if (buttons[i]->is_hovered) {
                render_rect.x -= 4;
                render_rect.y -= 2;
                render_rect.w += 8;
                render_rect.h += 4;
                if (buttons[i]->is_text) {
                    text_rect.x -= 2;
                    text_rect.y -= 1;
                    text_rect.w += 4;
                    text_rect.h += 2;
                }
            }

            // Afficher la texture
            SDL_RenderCopyEx(renderer, buttons[i]->texture, NULL, &render_rect, 0, NULL, SDL_FLIP_NONE);
            if (buttons[i]->is_text) SDL_RenderCopyEx(renderer, buttons[i]->text_texture, NULL, &text_rect, 0, NULL, SDL_FLIP_NONE);
        }
    }
}

Button* button_get(int id) {
    for (int i = 0; i < button_count; i++) {
        if (buttons[i] && buttons[i]->id == id) {
            return buttons[i];
        }
    }
    return NULL;
}

void buttons_free() {
    for (int i = 0; i < button_count; i++) {
        if (buttons[i]) {
            if (buttons[i]->is_text) SDL_DestroyTexture(buttons[i]->text_texture);
            SDL_DestroyTexture(buttons[i]->texture);
            free(buttons[i]);
            buttons[i] = NULL;
        }
    }
    button_count = 0;
}

void hide_button(int id) {
    Button* button = button_get(id);
    if (button) button->hidden = 1;
}

void show_button(int id) {
    Button* button = button_get(id);
    if (button) button->hidden = 0;
}