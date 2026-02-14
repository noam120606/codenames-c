#ifndef INPUT_H
#define INPUT_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../lib/sdl.h"


/**
 * Identifiants des inputs.
 */
typedef enum InputId {
    INPUT_NAME,
    INPUT_JOIN_CODE,
} InputId;


/**
 * Valeurs de retour des callbacks d'inputs.
 */
typedef enum InputReturn {
    INPUT_RET_NONE,
    INPUT_RET_NAME,
    INPUT_RET_JOIN_CODE,
} InputReturn;


/**
 * Typedef pour la fonction callback d'un input.
 * @param input_id L'ID de l'input qui a été modifié.
 */
typedef InputReturn (*InputCallback)(SDL_Context context, InputId input_id);


/**
 * Structure représentant un input avec callback.
 * @param id Identifiant unique de l'input.
 * @param buffer Buffer de texte pour stocker la saisie de l'utilisateur.
 * @param length Longueur actuelle du texte dans le buffer.
 * @param rect Rectangle définissant la position et la taille.
 * @param texture Texture de l'input.
 * @param is_hovered Indique si l'input est survolé.
 * @param is_text Indique si l'input est un input de texte.
 * @param text_rect Rectangle définissant la position et la taille du texte (UTF-8) => Seulement si is_text est vrai.
 * @param text_texture Texture du texte (UTF-8) => Seulement si is_text est vrai.
 * @param hidden Indique si l'input est caché (non affiché et non interact
 * @param callback Fonction à exécuter lors du clic.
 */
typedef struct {
    int id;
    char buffer[256];
    int length;
    SDL_Rect rect;
    SDL_Texture* texture;
    int is_hovered;
    int is_text;
    SDL_Rect text_rect;
    SDL_Texture* text_texture;
    int hidden;
    InputCallback callback;
} Input;


/**
 * Initialise le système d'inputs.
 * Doit être appelé une fois au démarrage.
 */
void inputs_init();


/**
 * Crée et ajoute un input au système.
 * @param id Identifiant unique de l'input.
 * @param x Position x de l'input.
 * @param y Position y de l'input.
 * @param w Largeur de l'input.
 * @param h Hauteur de l'input.
 * @param texture Texture de l'input.
 * @param callback Fonction à exécuter lors du clic (peut être NULL).
 * @return Pointeur vers l'input créé, ou NULL en cas d'erreur.
 */
Input* input_create(int id, int x, int y, int w, int h, SDL_Texture* texture, InputCallback callback);


/**
 * Crée et ajoute un input au système à partir d'un texte (utilise SDL_ttf).
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param id Identifiant unique de l'input.
 * @param x Position x de l'input.
 * @param y Position y de l'input.
 * @param taille Taille de l'input (sa hauteur).
 * @param text Texte à afficher sur l'input (UTF-8).
 * @param font_path Chemin vers le fichier .ttf.
 * @param color Couleur du texte.
 * @param callback Fonction à exécuter lors du clic (peut être NULL).
 * @return Pointeur vers l'input créé, ou NULL en cas d'erreur.
 */
Input* text_input_create(SDL_Renderer* renderer, int id, int x, int y, int taille,
                               const char* text, const char* font_path, SDL_Color color,
                               InputCallback callback);


/**
 * Traite un événement SDL pour les inputs.
 * @param event L'événement SDL à traiter.
 */
InputReturn inputs_handle_event(SDL_Context context, SDL_Event* event);


/**
 * Affiche tous les inputs.
 * @param renderer Le renderer SDL.
 */
void inputs_display(SDL_Renderer* renderer);


/**
 * Récupère un input par son ID.
 * @param id L'ID de l'input.
 * @return Pointeur vers l'input, ou NULL si non trouvé.
 */
Input* input_get(int id);


/**
 * Libère tous les inputs et le système.
 */
void inputs_free();


#endif // INPUT_H