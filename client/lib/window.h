/**
 * @file window.h
 * @brief Gestion des fenêtres UI (pop-ups, panneaux déplaçables).
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;
typedef struct Button Button;
typedef struct Input Input;
typedef struct Text Text;

#define MAX_WINDOWS 32

/**
 * Identifiants des différentes fenêtres.
 */
typedef enum {
    WINDOW_NONE = 0,
    WINDOW_MENU,
	WINDOW_TUTO,
	WINDOW_LOBBY_ROLE_NONE,
	WINDOW_LOBBY_BLUE_AGENTS,
	WINDOW_LOBBY_BLUE_SPY,
	WINDOW_LOBBY_RED_AGENTS,
	WINDOW_LOBBY_RED_SPY,
	WINDOW_LOBBY_GAME_OPTIONS,
	WINDOW_GAME_BLUE_PANEL,
	WINDOW_GAME_RED_PANEL,
	WINDOW_GAME_HISTORY_BLUE,
	WINDOW_GAME_HISTORY_RED,
	WINDOW_GAME_HINT,
	WINDOW_GAME_CHAT,
} WindowId;

/**
 * Configuration d'une fenêtre UI.
 * Les coordonnées x/y sont relatives au centre logique de l'écran.
 * @param x Position horizontale relative au centre de l'écran (en pixels).
 * @param y Position verticale relative au centre de l'écran (en pixels).
 * @param w Largeur de la fenêtre (en pixels).
 * @param h Hauteur de la fenêtre (en pixels).
 * @param movable Indique si la fenêtre peut être déplacée par l'utilisateur (1 = oui, 0 = non).
 * @param hidden Indique si la fenêtre est cachée (1 = oui, 0 = non).
 * @param bg_color Couleur de fond de la fenêtre (RGBA).
 * @param border_color Couleur de la bordure de la fenêtre (RGBA).
 * @param titlebar_color Couleur de la barre de titre de la fenêtre (RGBA).
 * @param window_texture Texture de fond de la fenêtre (NULL pour utiliser bg_color).
 * @param titlebar_texture Texture de la barre de titre (NULL pour utiliser titlebar_color).
 * @param title Titre affiché dans la barre de titre (NULL pour aucun titre).
 * @param border_thickness Épaisseur de la bordure de la fenêtre (en pixels).
 * @param titlebar_h Hauteur de la barre de titre de la fenêtre (en pixels).
 * @param scrollable Active la zone scrollable (1 = active, 0 = inactive).
 * @param scroll_x1 Coordonnée X du premier point de la zone scrollable (relative au centre de la fenêtre).
 * @param scroll_y1 Coordonnée Y du premier point de la zone scrollable (relative au centre de la fenêtre).
 * @param scroll_x2 Coordonnée X du second point de la zone scrollable (relative au centre de la fenêtre).
 * @param scroll_y2 Coordonnée Y du second point de la zone scrollable (relative au centre de la fenêtre).
 * @param scroll_offset Offset de scroll courant (unité logique libre selon l'écran).
 * @param scroll_min Valeur minimale autorisée pour scroll_offset.
 * @param scroll_max Valeur maximale autorisée pour scroll_offset.
 * @param scroll_step Pas appliqué à chaque cran de molette.
 */
typedef struct WindowConfig {
	int x;
	int y;
	int w;
	int h;
	int movable;
	int hidden;
	SDL_Color bg_color;
	SDL_Color border_color;
	SDL_Color titlebar_color;
	SDL_Texture* window_texture;
	SDL_Texture* titlebar_texture;
	char* title;
	SDL_Texture* title_texture_cache;
	int title_texture_cache_w;
	int title_texture_cache_h;
	int title_texture_cache_font_size;
	int title_texture_cache_dirty;
	int border_thickness;
	int titlebar_h;
	int scrollable;
	int scroll_x1;
	int scroll_y1;
	int scroll_x2;
	int scroll_y2;
	int scroll_offset;
	int scroll_min;
	int scroll_max;
	int scroll_step;

	/* Etat d'exécution */
	SDL_Rect rect;
	int dragging;
	int drag_offset_x;
	int drag_offset_y;
} WindowConfig;

typedef struct Window {
	int id;
	WindowConfig* cfg;
} Window;

/**
 * Clés de configuration modifiables avec `window_edit_cfg`.
 */
typedef enum WindowCfgKey {
	WIN_CFG_X = 100,
	WIN_CFG_Y,
	WIN_CFG_W,
	WIN_CFG_H,
	WIN_CFG_MOVABLE,
	WIN_CFG_HIDDEN,
	WIN_CFG_BG_COLOR,
	WIN_CFG_BORDER_COLOR,
	WIN_CFG_TITLEBAR_COLOR,
	WIN_CFG_WINDOW_TEXTURE,
	WIN_CFG_TITLEBAR_TEXTURE,
	WIN_CFG_TITLE,
	WIN_CFG_BORDER_THICKNESS,
	WIN_CFG_TITLEBAR_H,
	WIN_CFG_RECT,
	WIN_CFG_DRAGGING,
	WIN_CFG_DRAG_OFFSET_X,
	WIN_CFG_DRAG_OFFSET_Y,
	WIN_CFG_SCROLLABLE,
	WIN_CFG_SCROLL_X1,
	WIN_CFG_SCROLL_Y1,
	WIN_CFG_SCROLL_X2,
	WIN_CFG_SCROLL_Y2,
	WIN_CFG_SCROLL_OFFSET,
	WIN_CFG_SCROLL_MIN,
	WIN_CFG_SCROLL_MAX,
	WIN_CFG_SCROLL_STEP,
} WindowCfgKey;

/** Initialise une configuration de fenêtre avec des valeurs par défaut.
 * @return Un pointeur vers la configuration initialisée, ou NULL en cas d'erreur.
 */
WindowConfig* window_config_init();

/**
 * Crée une fenêtre à partir d'une configuration.
 * Si cfg est NULL, les valeurs par défaut sont utilisées.
 * @param id L'identifiant de la fenêtre.
 * @param cfg La configuration de la fenêtre.
 * @return Un pointeur vers la fenêtre créée, ou NULL en cas d'erreur.
 */
Window* window_create(int id, const WindowConfig* cfg);

/**
 * Détruit une fenêtre et libère ses ressources.
 * @param win La fenêtre à détruire.
 */
void window_destroy(Window* win);

/**
 * Recalcule le rectangle écran depuis x/y/w/h.
 * @param win La fenêtre à mettre à jour.
 */
void window_update_rect(Window* win);

/**
 * Gère les événements SDL (clic et drag) pour déplacer la fenêtre.
 * @param ctx Le contexte de l'application.
 * @param win La fenêtre à gérer.
 * @param event L'événement SDL à traiter.
 */
void window_handle_event(AppContext* ctx, Window* win, SDL_Event* event);

/**
 * Récupère une fenêtre par son identifiant.
 * @param id L'identifiant de la fenêtre à récupérer.
 * @return Un pointeur vers la fenêtre correspondante, ou NULL si aucune fenêtre avec cet ID n'existe.
 */
Window* window_get_by_id(int id);

/**
 * Dessine la fenêtre (fond, bordure, barre de titre).
 * @param renderer Le renderer SDL.
 * @param win La fenêtre à dessiner.
 */
void window_render(SDL_Renderer* renderer, const Window* win);

/**
 * Teste si un point logique (coordonnées SDL logiques) est dans la fenêtre.
 * @param win La fenêtre à tester.
 * @param logical_x Coordonnée x du point à tester (en pixels, relative au centre de l'écran).
 * @param logical_y Coordonnée y du point à tester (en pixels, relative au centre de l'écran).
 * @return 1 si le point est dans la fenêtre, 0 sinon.
 */
int window_contains_point(const Window* win, int logical_x, int logical_y);

/**
 * Modifie un champ de config d'une fenêtre.
 * @param win La fenêtre à modifier.
 * @param key La clé de configuration à modifier.
 * @param value La nouvelle valeur. Pour `WIN_CFG_WINDOW_TEXTURE` et
 * `WIN_CFG_TITLEBAR_TEXTURE`, fournir un pointeur `SDL_Texture*` casté en `intptr_t`.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int window_edit_cfg(Window* win, WindowCfgKey key, intptr_t value);

/**
 * Calcule le rectangle écran de la zone scrollable d'une fenêtre.
 * La zone est définie par deux points relatifs au centre de la fenêtre.
 * @param win Fenêtre de référence.
 * @param out_rect Reçoit le rectangle écran calculé.
 * @return EXIT_SUCCESS si la zone est valide et active, EXIT_FAILURE sinon.
 */
int window_get_scrollable_zone_rect(const Window* win, SDL_Rect* out_rect);

/**
 * Convertit une position locale de contenu de fenêtre en coordonnées logiques globales.
 * Le repère local est en pixels depuis le centre de la fenêtre,
 * avec x vers la droite et y vers le bas.
 * @param win La fenêtre de référence.
 * @param rel_x Position locale X (pixels).
 * @param rel_y Position locale Y (pixels).
 * @param out_logical_x Reçoit la coordonnée logique globale X (peut être NULL).
 * @param out_logical_y Reçoit la coordonnée logique globale Y (peut être NULL).
 */
void window_content_to_logical(const Window* win, int rel_x, int rel_y, int* out_logical_x, int* out_logical_y);

/**
 * Place un bouton dans la fenêtre avec une position locale (coin haut-gauche du bouton).
 * Avec rel_x=0 et rel_y=0, le bouton est aligné sur le centre de la fenêtre.
 * @param win Fenêtre de référence.
 * @param button Bouton à placer.
 * @param rel_x Position locale X (pixels).
 * @param rel_y Position locale Y (pixels).
 * @return EXIT_SUCCESS si succès, EXIT_FAILURE sinon.
 */
int window_place_button(const Window* win, Button* button, int rel_x, int rel_y);

/**
 * Place un input dans la fenêtre avec une position locale (coin haut-gauche de l'input).
 * Avec rel_x=0 et rel_y=0, l'input est aligné sur le centre de la fenêtre.
 * @param win Fenêtre de référence.
 * @param in Input à placer.
 * @param rel_x Position locale X (pixels).
 * @param rel_y Position locale Y (pixels).
 * @return EXIT_SUCCESS si succès, EXIT_FAILURE sinon.
 */
int window_place_input(const Window* win, Input* in, int rel_x, int rel_y);

/**
 * Place un texte dans la fenêtre avec une position locale (coin haut-gauche du texte).
 * Avec rel_x=0 et rel_y=0, le texte est aligné sur le centre de la fenêtre.
 * @param text Le texte à positionner (doit avoir une texture initialisée).
 * @return EXIT_SUCCESS si succès, EXIT_FAILURE sinon.
 */
int window_place_text(const Window* win, Text* text, int rel_x, int rel_y);

/**
 * Affiche une image à une position locale dans la fenêtre (coin haut-gauche de l'image rendue).
 * @param renderer Le renderer SDL.
 * @param win Fenêtre de référence.
 * @param texture Texture à afficher.
 * @param rel_x Position locale X (pixels).
 * @param rel_y Position locale Y (pixels).
 * @param size_factor Facteur d'échelle.
 * @param angle Angle de rotation en degrés.
 * @param flip Mode de flip SDL.
 * @param ratio Ratio largeur/hauteur appliqué par display_image.
 * @param opacity Opacité 0-255.
 * @return EXIT_SUCCESS si succès, EXIT_FAILURE sinon.
 */
int window_display_image(SDL_Renderer* renderer, const Window* win, SDL_Texture* texture, int rel_x, int rel_y, float size_factor, double angle, SDL_RendererFlip flip, float ratio, Uint8 opacity);

#endif /* WINDOW_H */
