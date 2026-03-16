#include "../lib/all.h"

static Button* buttons[MAX_BUTTONS];
static int button_count = 0;
static SDL_Renderer* button_renderer = NULL;

ButtonConfig* button_config_init(void) {
    ButtonConfig* cfg = (ButtonConfig*)calloc(1, sizeof(ButtonConfig));
    if (!cfg) return NULL;

    cfg->x = 0;
    cfg->y = 0;
    cfg->w = 0;
    cfg->h = 64;
    cfg->text = NULL;
    cfg->font_path = NULL;
    cfg->color = (SDL_Color){255, 255, 255, 255};
    cfg->tex_path = NULL;
    cfg->hidden = 0;
    cfg->callback = NULL;

    /* champs runtime */
    cfg->rect = (SDL_Rect){0, 0, 0, 0};
    cfg->texture = NULL;
    cfg->is_hovered = 0;
    cfg->is_text = 0;
    cfg->text_rect = (SDL_Rect){0, 0, 0, 0};
    cfg->text_texture = NULL;

    return cfg;
}

void buttons_init(SDL_Renderer* renderer) {
    button_renderer = renderer;
    for (int i = 0; i < MAX_BUTTONS; i++) {
        buttons[i] = NULL;
    }
    button_count = 0;
}

Button* button_create(SDL_Renderer* renderer, int id, const ButtonConfig* cfg_in) {
    if (button_count >= MAX_BUTTONS) {
        printf("button_create: nombre maximum de boutons atteint (%d)\n", MAX_BUTTONS);
        return NULL;
    }

    Button* button = (Button*)malloc(sizeof(Button));
    if (!button) {
        printf("button_create: allocation mémoire échouée pour le bouton %d\n", id);
        return NULL;
    }
    button->id = id;

    /* Le bouton possède sa propre copie de la config */
    button->cfg = button_config_init();
    if (!button->cfg) {
        free(button);
        return NULL;
    }
    if (cfg_in) {
        *button->cfg = *cfg_in;
    }

    /* Réinitialiser les champs runtime */
    button->cfg->is_hovered = 0;
    button->cfg->texture = NULL;
    button->cfg->text_texture = NULL;

    int x = button->cfg->x;
    int y = button->cfg->y;
    int btn_h = button->cfg->h;
    int btn_w = button->cfg->w;

    if (button->cfg->text != NULL) {
        /* --- Bouton texte --- */
        if (!renderer || !button->cfg->font_path) {
            printf("button_create: renderer ou font_path manquant pour un bouton texte (id=%d)\n", id);
            free(button->cfg);
            free(button);
            return NULL;
        }

        /* Rendre le texte */
        TTF_Font* font = TTF_OpenFont(button->cfg->font_path, 128);
        if (!font) {
            printf("button_create: TTF_OpenFont failed: %s\n", TTF_GetError());
            free(button->cfg);
            free(button);
            return NULL;
        }
        SDL_Surface* text_surf = TTF_RenderUTF8_Blended(font, button->cfg->text, button->cfg->color);
        TTF_CloseFont(font);
        if (!text_surf) {
            printf("button_create: TTF_RenderUTF8_Blended failed: %s\n", TTF_GetError());
            free(button->cfg);
            free(button);
            return NULL;
        }
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);
        int text_tex_w = text_surf->w;
        int text_tex_h = text_surf->h;
        SDL_FreeSurface(text_surf);
        if (!text_texture) {
            printf("button_create: SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
            free(button->cfg);
            free(button);
            return NULL;
        }

        /* Charger l'image de fond */
        const char* bg_path = button->cfg->tex_path ? button->cfg->tex_path : "assets/img/buttons/empty.png";
        SDL_Texture* background = load_image(renderer, bg_path);
        if (!background) {
            printf("button_create: load_image failed (%s): %s\n", bg_path, SDL_GetError());
            SDL_DestroyTexture(text_texture);
            free(button->cfg);
            free(button);
            return NULL;
        }

        int bg_tex_w = 0, bg_tex_h = 0;
        if (SDL_QueryTexture(background, NULL, NULL, &bg_tex_w, &bg_tex_h) != 0) {
            printf("button_create: SDL_QueryTexture failed: %s\n", SDL_GetError());
            SDL_DestroyTexture(background);
            SDL_DestroyTexture(text_texture);
            free(button->cfg);
            free(button);
            return NULL;
        }

        /* Calculer la largeur du bouton en conservant le ratio de l'image de fond */
        if (btn_w <= 0) {
            btn_w = (bg_tex_h > 0) ? (bg_tex_w * btn_h) / bg_tex_h : btn_h;
            if (btn_w <= 0) btn_w = btn_h;
        }

        /* Calculer le rectangle du texte centré dans le bouton */
        int padding = 16;
        SDL_Rect text_rect;
        text_rect.h = btn_h - 2 * padding;
        if (text_rect.h <= 0) text_rect.h = btn_h;

        double scale = (text_tex_h > 0) ? ((double)text_rect.h / (double)text_tex_h) : 1.0;
        text_rect.w = (int)(text_tex_w * scale);

        int max_text_w = btn_w - 2 * padding;
        if (text_rect.w > max_text_w) {
            if (text_tex_w > 0) {
                double scale2 = (double)max_text_w / (double)text_tex_w;
                text_rect.w = max_text_w;
                text_rect.h = (int)(text_tex_h * scale2);
                if (text_rect.h <= 0) text_rect.h = 1;
            } else {
                text_rect.w = max_text_w;
            }
        }
        button->cfg->texture = background;
        button->cfg->text_texture = text_texture;
        button->cfg->text_rect = (SDL_Rect){0, 0, text_rect.w, text_rect.h};
        button->cfg->is_text = 1;
    } else {
        /* --- Bouton image simple --- */
        if (button->cfg->tex_path) {
            if (!renderer) {
                printf("button_create: renderer manquant pour charger tex_path (id=%d)\n", id);
                free(button->cfg);
                free(button);
                return NULL;
            }
            button->cfg->texture = load_image(renderer, button->cfg->tex_path);
            if (!button->cfg->texture) {
                printf("button_create: load_image failed (%s): %s\n", button->cfg->tex_path, SDL_GetError());
                free(button->cfg);
                free(button);
                return NULL;
            }
        }
        button->cfg->is_text = 0;
    }

    /* Conversion coordonnées relatives au centre → écran.
     * x positif = droite, y positif = haut (convention mathématique).
     * (x, y) représente le centre du bouton relatif au centre de la fenêtre.
     * screen_x/screen_y sont les coordonnées SDL du coin haut-gauche. */
    int screen_x = (WIN_WIDTH  / 2) + x - (btn_w / 2);
    int screen_y = (WIN_HEIGHT / 2) - y - (btn_h / 2);

    button->cfg->x = screen_x;
    button->cfg->y = screen_y;
    button->cfg->rect = (SDL_Rect){screen_x, screen_y, btn_w, btn_h};
    button->cfg->w = btn_w;

    if (button->cfg->is_text) {
        button->cfg->text_rect.x = screen_x + (btn_w - button->cfg->text_rect.w) / 2;
        button->cfg->text_rect.y = screen_y + (btn_h - button->cfg->text_rect.h) / 2;
    }

    buttons[button_count++] = button;
    return button;
}

static int is_mouse_over_button(Button* button, int mouseX, int mouseY) {
    if (!button || !button->cfg) return 0;
    return (mouseX >= button->cfg->rect.x &&
            mouseX <= button->cfg->rect.x + button->cfg->rect.w &&
            mouseY >= button->cfg->rect.y &&
            mouseY <= button->cfg->rect.y + button->cfg->rect.h);
}

ButtonReturn buttons_handle_event(SDL_Context* context, SDL_Event* event) {
    if (!event) return BTN_RET_NONE;

    if (event->type == SDL_MOUSEMOTION) {
        int mouseX = event->motion.x;
        int mouseY = event->motion.y;
        for (int i = 0; i < button_count; i++) {
            if (!buttons[i] || !buttons[i]->cfg) continue;
            if (!buttons[i]->cfg->hidden) {
                buttons[i]->cfg->is_hovered = is_mouse_over_button(buttons[i], mouseX, mouseY);
            } else {
                buttons[i]->cfg->is_hovered = 0;
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mouseX = event->button.x;
        int mouseY = event->button.y;
        for (int i = 0; i < button_count; i++) {
            if (buttons[i] && buttons[i]->cfg &&
                !buttons[i]->cfg->hidden &&
                is_mouse_over_button(buttons[i], mouseX, mouseY)) {
                if (buttons[i]->cfg->callback) {
                    return buttons[i]->cfg->callback(context, buttons[i]->id);
                }
            }
        }
    }
    /* Si aucun bouton n'a été cliqué, retourner BTN_RET_NONE */
    return BTN_RET_NONE;
}

int button_render(ButtonId id) {

    Button* button = button_get(id);

    if (!button || !button->cfg) return EXIT_FAILURE;
    ButtonConfig* cfg = button->cfg;
    if (cfg->hidden || !cfg->texture) return EXIT_FAILURE;

    /* Grandir le bouton au survol */
    SDL_Rect render_rect = cfg->rect;
    SDL_Rect text_rect = cfg->text_rect;
    if (cfg->is_hovered) {
        render_rect.x -= 4;
        render_rect.y -= 2;
        render_rect.w += 8;
        render_rect.h += 4;
        if (cfg->is_text) {
            text_rect.x -= 2;
            text_rect.y -= 1;
            text_rect.w += 4;
            text_rect.h += 2;
        }
    }

    SDL_RenderCopyEx(button_renderer, cfg->texture, NULL, &render_rect, 0, NULL, SDL_FLIP_NONE);
    if (cfg->is_text) {
        SDL_RenderCopyEx(button_renderer, cfg->text_texture, NULL, &text_rect, 0, NULL, SDL_FLIP_NONE);
    }

    return EXIT_SUCCESS;
}

Button* button_get(int id) {
    for (int i = 0; i < button_count; i++) {
        if (buttons[i] && buttons[i]->id == id) return buttons[i];
    }
    return NULL;
}

int buttons_free(void) {
    for (int i = 0; i < button_count; i++) {
        if (buttons[i]) {
            if (buttons[i]->cfg) {
                if (buttons[i]->cfg->is_text && buttons[i]->cfg->text_texture) {
                    SDL_DestroyTexture(buttons[i]->cfg->text_texture);
                }
                if (buttons[i]->cfg->texture) {
                    SDL_DestroyTexture(buttons[i]->cfg->texture);
                }
                free(buttons[i]->cfg);
            }
            free(buttons[i]);
            buttons[i] = NULL;
        }
    }
    button_count = 0;
    return EXIT_SUCCESS;
}

int edit_btn_cfg(int id, ButtonCfgKey key, intptr_t value) {
    Button* button = button_get(id);
    if (!button || !button->cfg) return EXIT_FAILURE;

    switch (key) {
        case BTN_CFG_X:
            button->cfg->x = (int)value;
            button->cfg->rect.x = (int)value;
            return EXIT_SUCCESS;
        case BTN_CFG_Y:
            button->cfg->y = (int)value;
            button->cfg->rect.y = (int)value;
            return EXIT_SUCCESS;
        case BTN_CFG_W:
            button->cfg->w = (int)value;
            button->cfg->rect.w = (int)value;
            return EXIT_SUCCESS;
        case BTN_CFG_H:
            button->cfg->h = (int)value;
            button->cfg->rect.h = (int)value;
            return EXIT_SUCCESS;
        case BTN_CFG_TEXT:
            button->cfg->text = (const char*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_FONT_PATH:
            button->cfg->font_path = (const char*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_COLOR:
            if (!value) return EXIT_FAILURE;
            button->cfg->color = *(const SDL_Color*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_TEX_PATH:
            button->cfg->tex_path = (const char*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_HIDDEN:
            button->cfg->hidden = ((int)value != 0);
            return EXIT_SUCCESS;
        case BTN_CFG_CALLBACK:
            if (!value) return EXIT_FAILURE;
            button->cfg->callback = *(ButtonCallback*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_RECT:
            if (!value) return EXIT_FAILURE;
            button->cfg->rect = *(const SDL_Rect*)value;
            button->cfg->x = button->cfg->rect.x;
            button->cfg->y = button->cfg->rect.y;
            button->cfg->w = button->cfg->rect.w;
            button->cfg->h = button->cfg->rect.h;
            return EXIT_SUCCESS;
        case BTN_CFG_TEXTURE:
            button->cfg->texture = (SDL_Texture*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_IS_HOVERED:
            button->cfg->is_hovered = ((int)value != 0);
            return EXIT_SUCCESS;
        case BTN_CFG_IS_TEXT:
            button->cfg->is_text = ((int)value != 0);
            return EXIT_SUCCESS;
        case BTN_CFG_TEXT_RECT:
            if (!value) return EXIT_FAILURE;
            button->cfg->text_rect = *(const SDL_Rect*)value;
            return EXIT_SUCCESS;
        case BTN_CFG_TEXT_TEXTURE:
            button->cfg->text_texture = (SDL_Texture*)value;
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
    }
}
