
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "../SDL2/include/SDL2/SDL.h"
#include "../lib/sdl.h"

#define MAX_BUTTONS 100

/**
 * Identifiants des boutons.
 */
typedef enum ButtonId {
    BTN_NONE,
    BTN_CREATE,
    BTN_JOIN,
    BTN_QUIT,
    BTN_RED_AGENT,
    BTN_RED_SPY,
    BTN_BLUE_AGENT,
    BTN_BLUE_SPY,
    BTN_RETURN,
} ButtonId;

/**
 * Valeurs de retour des callbacks de boutons.
 */
typedef enum ButtonReturn {
    BTN_RET_NONE,
    BTN_RET_QUIT,
} ButtonReturn;

/**
 * Typedef pour la fonction callback d'un bouton.
 * @param button_id L'ID du bouton qui a été cliqué.
 */
typedef ButtonReturn (*ButtonCallback)(SDL_Context* context, ButtonId button_id);

/**
 * Configuration pour créer un `Button`.
 * Tous les champs ont des valeurs par défaut définies via `button_config_init`.
 * La structure peut être modifiée à tout moment et passée à `button_create`.
 *
 * Champs configurables :
 * @param x         Offset horizontal relatif au centre de la fenêtre (pixels).
 *                  Positif → vers la droite, négatif → vers la gauche.
 * @param y         Offset vertical relatif au centre de la fenêtre (pixels).
 *                  Positif → vers le haut, négatif → vers le bas.
 *                  Si x = 0 et y = 0, le centre du bouton est au centre de la fenêtre.
 *                  Formules appliquées dans button_create :
 *                    screen_x = WIN_WIDTH/2  + x - w/2
 *                    screen_y = WIN_HEIGHT/2 - y - h/2
 * @param w         Largeur du bouton. Si 0 et text != NULL, calculée automatiquement selon le ratio de l'image de fond.
 * @param h         Hauteur du bouton (et hauteur cible pour le texte si is_text).
 * @param text      Texte à afficher sur le bouton (UTF-8). NULL = bouton image sans texte.
 * @param font_path Chemin vers le fichier .ttf utilisé pour rendre le texte. Ignoré si text est NULL.
 * @param color     Couleur du texte. Ignorée si text est NULL.
 * @param tex_path  Chemin vers l'image de fond (assets/img/…). NULL = pas de fond personnalisé (utilise "assets/img/buttons/empty.png" par défaut si text != NULL).
 * @param hidden    Si 1, le bouton est caché (non affiché et non interactif).
 * @param callback  Fonction appelée lors du clic. Peut être NULL.
 *
 * Champs d'état (runtime, gérés en interne — ne pas modifier directement) :
 * @param rect         Rectangle de rendu calculé depuis x/y/w/h.
 * @param texture      Texture de fond chargée depuis tex_path.
 * @param is_hovered   Mis à jour automatiquement lors des événements souris.
 * @param is_text      Mis à 1 automatiquement si text != NULL.
 * @param text_rect    Rectangle de rendu du texte, calculé automatiquement.
 * @param text_texture Texture du texte, générée automatiquement depuis text/font_path/color.
*/
typedef struct ButtonConfig {
    /* --- champs configurables --- */
    int x;
    int y;
    int w;
    int h;
    const char* text;
    const char* font_path;
    SDL_Color color;
    const char* tex_path;
    int hidden;
    ButtonCallback callback;

    /* --- champs d'état (runtime) --- */
    SDL_Rect rect;
    SDL_Texture* texture;
    int is_hovered;
    int is_text;
    SDL_Rect text_rect;
    SDL_Texture* text_texture;
} ButtonConfig;

/**
 * Clés de configuration modifiables via `edit_btn_cfg`.
 */
typedef enum ButtonCfgKey {
    BTN_CFG_X = 100,
    BTN_CFG_Y,
    BTN_CFG_W,
    BTN_CFG_H,
    BTN_CFG_TEXT,
    BTN_CFG_FONT_PATH,
    BTN_CFG_COLOR,
    BTN_CFG_TEX_PATH,
    BTN_CFG_HIDDEN,
    BTN_CFG_CALLBACK,
    BTN_CFG_RECT,
    BTN_CFG_TEXTURE,
    BTN_CFG_IS_HOVERED,
    BTN_CFG_IS_TEXT,
    BTN_CFG_TEXT_RECT,
    BTN_CFG_TEXT_TEXTURE,
} ButtonCfgKey;

/**
 * Structure représentant un bouton.
 * @param id  Identifiant unique du bouton.
 * @param cfg Pointeur vers la configuration (possédée par le bouton, libérée par buttons_free).
 */
typedef struct {
    int id;
    ButtonConfig* cfg;
} Button;

/**
 * Initialise une ButtonConfig avec des valeurs par défaut.
 * @return Pointeur vers la config allouée, ou NULL en cas d'échec.
 */
ButtonConfig* button_config_init(void);

/**
 * Initialise le système de boutons.
 * Doit être appelé une fois au démarrage.
 */
void buttons_init(void);

/**
 * Crée et enregistre un bouton à partir d'une configuration.
 * Si cfg est NULL, les valeurs par défaut sont utilisées.
 * La configuration est copiée : le pointeur cfg peut être libéré après l'appel.
 * @param renderer Renderer SDL utilisé pour charger les textures.
 * @param id       Identifiant unique du bouton.
 * @param cfg      Pointeur vers la configuration. NULL = valeurs par défaut.
 * @return Pointeur vers le bouton créé, ou NULL en cas d'erreur.
 */
Button* button_create(SDL_Renderer* renderer, int id, const ButtonConfig* cfg);

/**
 * Traite un événement SDL pour les boutons.
 * @param context Contexte SDL.
 * @param event   L'événement SDL à traiter.
 */
ButtonReturn buttons_handle_event(SDL_Context* context, SDL_Event* event);

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
 * Libère tous les boutons et réinitialise le système.
 */
int buttons_free(void);

/**
 * Modifie une valeur de configuration d'un bouton.
 * @param id    L'ID du bouton à modifier.
 * @param key   Champ ciblé dans la configuration.
 * @param value Valeur à appliquer (entier direct pour champs int/bool, pointeur casté en `intptr_t` pour les champs pointeurs, et adresse d'une structure pour `BTN_CFG_COLOR` / `BTN_CFG_RECT`).
 * @return EXIT_SUCCESS si le bouton existe, sinon EXIT_FAILURE.
 */
int edit_btn_cfg(int id, ButtonCfgKey key, intptr_t value);

#endif // BUTTON_H
