/**
 * @file button.h
 * @brief Gestion des boutons interactifs de l'interface graphique.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;

/**
 * Valeurs de retour des callbacks de boutons.
 */
typedef enum ButtonReturn {
    BTN_RET_NONE,
    BTN_RET_QUIT,
} ButtonReturn;

/* Déclaration anticipée pour le typedef de callback. */
typedef struct Button Button;

/**
 * Typedef pour la fonction callback d'un bouton.
 * @param context Contexte SDL.
 * @param button  Le bouton qui a été cliqué.
 */
typedef ButtonReturn (*ButtonCallback)(AppContext* context, Button* button);

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
 * @param callback  Fonction appelée lors du clic. Peut être NULL.
 *
 * Champs d'état (runtime, gérés en interne — ne pas modifier directement) :
 * @param rect         Rectangle de rendu calculé depuis x/y/w/h.
 * @param texture      Texture de fond chargée depuis tex_path.
 * @param is_hovered   Mis à jour automatiquement lors des événements souris.
 * @param is_pressed   Mis à 1 quand un clic gauche commence sur le bouton.
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
    ButtonCallback callback;

    /* --- champs d'état (runtime) --- */
    SDL_Rect rect;
    SDL_Texture* texture;
    int is_hovered;
    int is_pressed;
    int is_text;
    SDL_Rect text_rect;
    SDL_Texture* text_texture;
    SDL_Renderer* renderer;
    int text_dirty;
} ButtonConfig;

/**
 * Clés de configuration modifiables via `button_edit_cfg`.
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
    BTN_CFG_CALLBACK,
    BTN_CFG_RECT,
    BTN_CFG_TEXTURE,
    BTN_CFG_IS_HOVERED,
    BTN_CFG_IS_PRESSED,
    BTN_CFG_IS_TEXT,
    BTN_CFG_TEXT_RECT,
    BTN_CFG_TEXT_TEXTURE,
} ButtonCfgKey;

/**
 * Structure représentant un bouton.
 * @param id  Identifiant libre (pour usage dans les callbacks, comparaisons, etc.).
 * @param cfg Pointeur vers la configuration (possédée par le bouton, libérée par button_destroy).
 */
struct Button {
    int id;
    ButtonConfig* cfg;
};

/**
 * Initialise une ButtonConfig avec des valeurs par défaut.
 * @return Pointeur vers la config allouée, ou NULL en cas d'échec.
 */
ButtonConfig* button_config_init(void);

/**
 * Crée un bouton à partir d'une configuration.
 * Si cfg est NULL, les valeurs par défaut sont utilisées.
 * La configuration est copiée : le pointeur cfg peut être libéré après l'appel.
 * Le bouton créé doit être détruit avec `button_destroy` pour éviter les fuites mémoire.
 * @param renderer Renderer SDL utilisé pour charger les textures.
 * @param id       Identifiant libre du bouton (pour usage dans les callbacks).
 * @param cfg      Pointeur vers la configuration. NULL = valeurs par défaut.
 * @return Pointeur vers le bouton créé, ou NULL en cas d'erreur.
 */
Button* button_create(SDL_Renderer* renderer, int id, const ButtonConfig* cfg);

/**
 * Détruit un bouton et libère ses ressources (textures, config).
 * @param button Pointeur vers le bouton à détruire. Si NULL, ne fait rien.
 */
void button_destroy(Button* button);

/**
 * Traite un événement SDL pour un bouton donné.
 * Gère le survol (hover) et le clic (appel du callback).
 * @param context Contexte SDL.
 * @param button  Pointeur vers le bouton à traiter.
 * @param event   L'événement SDL à traiter.
 * @return La valeur de retour du callback si le bouton a été cliqué, sinon BTN_RET_NONE.
 */
ButtonReturn button_handle_event(AppContext* context, Button* button, SDL_Event* event);

/**
 * Rendu d'un bouton à l'écran.
 * @param renderer Renderer SDL.
 * @param button   Pointeur vers le bouton à afficher.
 * @return EXIT_SUCCESS si le rendu a réussi, sinon EXIT_FAILURE.
 */
int button_render(SDL_Renderer* renderer, Button* button);

/**
 * Modifie un champ de configuration d'un bouton.
 * @param button Le bouton à modifier.
 * @param key Le champ de configuration ciblé.
 * @param value Nouvelle valeur (entier direct pour les champs int/bool,
 * pointeur casté en `intptr_t` pour les champs pointeurs et adresses de
 * structures pour `BTN_CFG_COLOR` / `BTN_CFG_RECT` / `BTN_CFG_TEXT_RECT`).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int button_edit_cfg(Button* button, ButtonCfgKey key, intptr_t value);

#endif // BUTTON_H
