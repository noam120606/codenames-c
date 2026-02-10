#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "../lib/button.h"
#include "../lib/sdl.h"

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

ButtonReturn buttons_handle_event(SDL_Context context, SDL_Event* event) {
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
}

void buttons_display(SDL_Renderer* renderer) {
    if (!renderer) {
        return;
    }

    for (int i = 0; i < button_count; i++) {
        if (buttons[i] && !buttons[i]->hidden && buttons[i]->texture) {

            // Grandir le bouton au survol
            SDL_Rect render_rect = buttons[i]->rect;
            if (buttons[i]->is_hovered) {
                render_rect.x -= 4;
                render_rect.y -= 2;
                render_rect.w += 8;
                render_rect.h += 4;
            }

            // Afficher la texture
            SDL_RenderCopy(renderer, buttons[i]->texture, NULL, &render_rect);
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