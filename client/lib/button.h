#ifndef BUTTON_H
#define BUTTON_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../lib/sdl.h"

/**
 * Identifiants des boutons.
 */
typedef enum ButtonId {
    BTN_JOIN,
    BTN_CREATE,
    BTN_QUIT
} ButtonId;

/**
 * Valeurs de retour des callbacks de boutons.
 */
typedef enum ButtonReturn {
    BTN_RET_NONE,
    BTN_RET_QUIT
} ButtonReturn;

/**
 * Typedef pour la fonction callback d'un bouton.
 * @param button_id L'ID du bouton qui a été cliqué.
 */
typedef ButtonReturn (*ButtonCallback)(SDL_Context context, ButtonId button_id);

/**
 * Structure représentant un bouton avec callback.
 * @param id Identifiant unique du bouton.
 * @param rect Rectangle définissant la position et la taille.
 * @param texture Texture du bouton.
 * @param is_hovered Indique si le bouton est survolé.
 * @param is_text Indique si le bouton est un bouton de texte.
 * @param text_rect Rectangle définissant la position et la taille du texte (UTF-8) => Seulement si is_text est vrai.
 * @param text_texture Texture du texte (UTF-8) => Seulement si is_text est vrai.
 * @param hidden Indique si le bouton est caché (non affiché et non interact
 * @param callback Fonction à exécuter lors du clic.
 */
typedef struct {
    int id;
    SDL_Rect rect;
    SDL_Texture* texture;
    int is_hovered;
    int is_text;
    SDL_Rect text_rect;
    SDL_Texture* text_texture;
    int hidden;
    ButtonCallback callback;
} Button;

/**
 * Initialise le système de boutons.
 * Doit être appelé une fois au démarrage.
 */
void buttons_init();

/**
 * Crée et ajoute un bouton au système.
 * @param id Identifiant unique du bouton.
 * @param x Position x du bouton.
 * @param y Position y du bouton.
 * @param w Largeur du bouton.
 * @param h Hauteur du bouton.
 * @param texture Texture du bouton.
 * @param callback Fonction à exécuter lors du clic (peut être NULL).
 * @return Pointeur vers le bouton créé, ou NULL en cas d'erreur.
 */
Button* button_create(int id, int x, int y, int w, int h, SDL_Texture* texture, ButtonCallback callback);

/**
 * Crée et ajoute un bouton au système à partir d'un texte (utilise SDL_ttf).
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param id Identifiant unique du bouton.
 * @param x Position x du bouton.
 * @param y Position y du bouton.
 * @param taille Taille du bouton (sa hauteur).
 * @param text Texte à afficher sur le bouton (UTF-8).
 * @param font_path Chemin vers le fichier .ttf.
 * @param color Couleur du texte.
 * @param callback Fonction à exécuter lors du clic (peut être NULL).
 * @return Pointeur vers le bouton créé, ou NULL en cas d'erreur.
 */
Button* text_button_create(SDL_Renderer* renderer, int id, int x, int y, int taille,
                               const char* text, const char* font_path, SDL_Color color,
                               ButtonCallback callback);

/**
 * Traite un événement SDL pour les boutons.
 * @param event L'événement SDL à traiter.
 */
ButtonReturn buttons_handle_event(SDL_Context context, SDL_Event* event);

/**
 * Affiche tous les boutons.
 * @param renderer Le renderer SDL.
 */
void buttons_display(SDL_Renderer* renderer);

/**
 * Récupère un bouton par son ID.
 * @param id L'ID du bouton.
 * @return Pointeur vers le bouton, ou NULL si non trouvé.
 */
Button* button_get(int id);

/**
 * Libère tous les boutons et le système.
 */
void buttons_free();

#endif // BUTTON_H
