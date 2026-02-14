#include "../lib/all.h"

#define MAX_INPUTS 50

static Input* inputs[MAX_INPUTS];
static int input_count = 0;

void inputs_init() {
    for (int i = 0; i < MAX_INPUTS; i++) {
        inputs[i] = NULL;
    }
    input_count = 0;
}


Input* input_create(int id, int x, int y, int w, int h, SDL_Texture* texture, InputCallback callback) {
    if (input_count >= MAX_INPUTS) {
        printf("Erreur: nombre maximum d'inputs atteint (%d)\n", MAX_INPUTS);
        return NULL;
    }

    if (!texture) {
        printf("Erreur: texture NULL pour le bouton %d\n", id);
        return NULL;
    }

    Input* input = (Input*)malloc(sizeof(Input));
    if (!input) {
        printf("Erreur: allocation mémoire échouée pour l'input %d\n", id);
        return NULL;
    }

    input->id = id;
    input->buffer[0] = '\0';
    input->length = 0;
    input->rect.x = x;
    input->rect.y = y;
    input->rect.w = w;
    input->rect.h = h;
    input->texture = texture;
    input->is_hovered = 0;
    input->is_text = 0;
    input->hidden = 0;
    input->callback = callback;

    inputs[input_count++] = input;

    return input;
}

Input* text_input_create(SDL_Renderer* renderer, int id, int x, int y, int taille,
                            const char* text, const char* font_path, SDL_Color color,
                            InputCallback callback) {

    if (!renderer || !text || !font_path) {
        printf("text_input_create: invalid argument\n");
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

    SDL_Texture* background = load_image(renderer, "assets/img/inputs/empty.png");
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

    Input* input = (Input*)malloc(sizeof(Input));
    if (!input) {
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

    input->id = id;
    input->rect = (SDL_Rect){x, y, btn_w, btn_h};
    input->texture = background;
    input->is_hovered = 0;
    input->is_text = 1;
    input->text_rect = text_rect;
    input->text_texture = text_texture;
    input->hidden = 0;
    input->callback = callback;

    inputs[input_count++] = input;

    return input;
}

static int is_mouse_over_input(Input* input, int mouseX, int mouseY) {
    if (!input) {
        return 0;
    }
    return (mouseX >= input->rect.x && 
            mouseX <= input->rect.x + input->rect.w &&
            mouseY >= input->rect.y && 
            mouseY <= input->rect.y + input->rect.h);
}

InputReturn input_handle_event(SDL_Context context, SDL_Event* event) {
    if (!event) {
        return INPUT_RET_NONE;
    }

    if (event->type == SDL_MOUSEMOTION) {
        int mouseX = event->motion.x;
        int mouseY = event->motion.y;

        // Mettre à jour l'état de survol pour tous les inputs
        for (int i = 0; i < input_count; i++) {
            if (inputs[i] && !inputs[i]->hidden) {
                inputs[i]->is_hovered = is_mouse_over_input(inputs[i], mouseX, mouseY);
            } else if (inputs[i]) {
                inputs[i]->is_hovered = 0; // Ne pas surligner les inputs cachés
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mouseX = event->button.x;
        int mouseY = event->button.y;

        // Vérifier et déclencher les callbacks des inputs cliqués
        for (int i = 0; i < input_count; i++) {

            if (inputs[i] && !inputs[i]->hidden && is_mouse_over_input(inputs[i], mouseX, mouseY)) {
                if (inputs[i]->callback) {
                    return inputs[i]->callback(context, inputs[i]->id);
                }
            }
        }
    }
    // Si aucun input n'a été cliqué, retourner INPUT_RET_NONE (avec un oubli ça avait tout cassé)
    return INPUT_RET_NONE;
}

void inputs_display(SDL_Renderer* renderer) {
    if (!renderer) {
        return;
    }

    for (int i = 0; i < input_count; i++) {
        if (inputs[i] && !inputs[i]->hidden && inputs[i]->texture) {

            // Grandir l'input au survol
            SDL_Rect render_rect = inputs[i]->rect;
            SDL_Rect text_rect = inputs[i]->text_rect;
            if (inputs[i]->is_hovered) {
                render_rect.x -= 4;
                render_rect.y -= 2;
                render_rect.w += 8;
                render_rect.h += 4;
                if (inputs[i]->is_text) {
                    text_rect.x -= 2;
                    text_rect.y -= 1;
                    text_rect.w += 4;
                    text_rect.h += 2;
                }
            }

            // Afficher la texture
            SDL_RenderCopy(renderer, inputs[i]->texture, NULL, &render_rect);
            if (inputs[i]->is_text) SDL_RenderCopy(renderer, inputs[i]->text_texture, NULL, &text_rect);
        }
    }
}

Input* input_get(int id) {
    for (int i = 0; i < input_count; i++) {
        if (inputs[i] && inputs[i]->id == id) {
            return inputs[i];
        }
    }
    return NULL;
}

void inputs_free() {
    for (int i = 0; i < input_count; i++) {
        if (inputs[i]) {
            if (inputs[i]->is_text) SDL_DestroyTexture(inputs[i]->text_texture);
            SDL_DestroyTexture(inputs[i]->texture);
            free(inputs[i]);
            inputs[i] = NULL;
        }
    }
    input_count = 0;
}

void hide_input(int id) {
    Input* input = input_get(id);
    if (input) input->hidden = 1;
}

void show_input(int id) {
    Input* input = input_get(id);
    if (input) input->hidden = 0;
}