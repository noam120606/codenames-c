#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include "../SDL2/include/SDL2/SDL.h"

typedef struct AppContext AppContext;

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
 * @param title Titre affiché dans la barre de titre (NULL pour aucun titre).
 * @param border_thickness Épaisseur de la bordure de la fenêtre (en pixels).
 * @param titlebar_h Hauteur de la barre de titre de la fenêtre (en pixels).
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
	char* title;
	int border_thickness;
	int titlebar_h;

	/* Etat runtime */
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
	WIN_CFG_TITLE,
	WIN_CFG_BORDER_THICKNESS,
	WIN_CFG_TITLEBAR_H,
	WIN_CFG_RECT,
	WIN_CFG_DRAGGING,
	WIN_CFG_DRAG_OFFSET_X,
	WIN_CFG_DRAG_OFFSET_Y,
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
 * @param value La nouvelle valeur.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int window_edit_cfg(Window* win, WindowCfgKey key, intptr_t value);

#endif /* WINDOW_H */
