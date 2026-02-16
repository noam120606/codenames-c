#ifndef INPUT_H
#define INPUT_H

#include "sdl.h"

#define INPUT_DEFAULT_MAX 256

typedef enum {
    INPUT_ID_NONE = 0,
    INPUT_ID_NAME,
    INPUT_ID_JOIN_CODE,
    INPUT_ID_TCHAT
} InputId;

typedef struct Input {
    SDL_Rect rect;
    char* text;
    int maxlen;
    int len;
    int cursor_pos;
    int focused;
    int submitted;
    SDL_Color bg_color;
    SDL_Color border_color;
    SDL_Color text_color;
    SDL_Texture* bg_texture;
    int padding;
    InputId id;
    int sel_start;
    int sel_len;
    const char* font_path;
    int font_size;
    void (*on_submit)(const char*);
} Input;

/**
 * Crée un nouvel input.
 * @param x Position x de l'input.
 * @param y Position y de l'input.
 * @param w Largeur de l'input.
 * @param h Hauteur de l'input.
 * @param font_path Chemin vers le fichier de police (.otf/.ttf) pour le texte
 * @param font_size Taille de la police.
 * @param maxlen Longueur maximale du texte (excluant le caractère nul). Si <= 0, une valeur par défaut est utilisée.
 * @return Pointeur vers l'input créé, ou NULL en cas d'erreur.
 */
Input* input_create(InputId id, int x, int y, int w, int h, const char* font_path, int font_size, int maxlen);

/** Détruit un input. 
 * Libère la mémoire associée à l'input et ses ressources (texte, texture de fond).
 * @param in Pointeur vers l'input à détruire. Si NULL, la fonction ne fait rien.
 */
void input_destroy(Input* in);

/**
 * Gère les événements SDL pour l'input (clics, saisie de texte, etc.).
 * Doit être appelé à chaque événement SDL dans la boucle principale.
 * @param in Pointeur vers l'input à gérer.
 * @param e Pointeur vers l'événement SDL à traiter.
 */
void input_handle_event(Input* in, SDL_Event* e);

/** Affiche l'input sur le renderer.
 * Doit être appelé à chaque frame dans la boucle de rendu.
 * @param renderer Le renderer SDL sur lequel dessiner l'input.
 * @param in Pointeur vers l'input à afficher.
 */
void input_render(SDL_Renderer* renderer, Input* in);

/**
 * Récupère le texte actuel de l'input.
 * @param in Pointeur vers l'input.
 * @return Le texte actuel de l'input, ou NULL si l'input est NULL
 */
const char* input_get_text(Input* in);

/**
 * Vérifie si l'input a été soumis (par exemple, en appuyant sur Entrée).
 * @param in Pointeur vers l'input.
 * @return 1 si l'input a été soumis, 0 sinon ou si l'input est NULL.
 */
int input_is_submitted(Input* in);

/**
 * Efface l'état de "soumis" de l'input. Doit être appelé après avoir traité une soumission pour réinitialiser l'état.
 * @param in Pointeur vers l'input.
 */
void input_clear_submitted(Input* in);

/**
 * Définit le texte de l'input. Si le texte dépasse la longueur maximale, il sera tronqué.
 * @param in Pointeur vers l'input.
 */
void input_set_text(Input* in, const char* text);

/**
 * Définit une fonction de rappel à appeler lorsque l'input est soumis (par exemple, en appuyant sur Entrée).
 * @param in Pointeur vers l'input.
 * @param cb Pointeur vers la fonction de rappel à appeler lors de la soumission. La fonction doit prendre un const char* (le texte soumis) et retourner void. Si NULL, aucune fonction ne sera appelée.
 */
void input_set_on_submit(Input* in, void (*cb)(const char*));

/**
 * Définit une texture de fond pour l'input à partir d'une image. Si une texture de fond existe déjà, elle sera remplacée.
 * @param in Pointeur vers l'input.
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param path Chemin vers l'image à utiliser comme fond.
 * @param padding Si >= 0, définit le padding intérieur entre le texte et les bords de l'input. Si < 0, le padding sera automatiquement ajusté en fonction de la taille de l'image de fond.
 * @return `EXIT_SUCCESS` si la texture a été définie avec succès, `EXIT_FAILURE` en cas d'erreur (par exemple, si le chargement de l'image échoue).
 */
int input_set_bg(Input* in, SDL_Renderer* renderer, const char* path, int padding);

/**
 * Efface la texture de fond de l'input, revenant à un fond uni. Si aucune texture de fond n'est définie, cette fonction ne fait rien.
 * @param in Pointeur vers l'input.
 */
void input_clear_bg(Input* in);

/**
 * Définit le padding intérieur de l'input, c'est-à-dire l'espace entre le texte et les bords de l'input. Si une texture de fond est définie, il est recommandé d'utiliser `input_set_bg` avec un padding approprié pour assurer une bonne apparence.
 * @param in Pointeur vers l'input.
 * @param padding Nouvelle valeur de padding en pixels. Si < 0, le padding sera défini à 0.
 */
void input_set_padding(Input* in, int padding);

#endif /* INPUT_H */
