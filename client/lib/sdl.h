#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080

/**
 * Contexte SDL contenant la fenêtre et le renderer.
 * @param window Fenêtre SDL.
 * @param renderer Renderer SDL associé à la fenêtre.
 * @param clock Nombre de frame écoulé depuis le début de l'application.
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    long clock;
} SDL_Context;

/**
 * Initialise SDL et crée une fenêtre et un renderer.
 * @return Un SDL_Context contenant la fenêtre et le renderer,
 *         ou un SDL_Context avec des pointeurs `NULL` en cas d'erreur.
 */
SDL_Context init_sdl();

/**
 * Charge une image depuis le disque et retourne une texture SDL.
 * @param renderer Renderer SDL.
 * @param path Chemin de l'image.
 * @return Texture SDL, ou `NULL` en cas d'erreur.
 */
SDL_Texture* load_image(SDL_Renderer* renderer, const char* path);

/**
 * Affiche une texture à l'écran.
 * @param renderer Le renderer SDL
 * @param texture La texture à afficher
 * @param x La position x de l'image
 * @param y La position y de l'image
 * @param w La largeur de l'image (peut être `0` pour garder la valeur originale)
 * @param h La hauteur de l'image (peut être `0` pour garder la valeur originale)
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur
 * Note : Si une seule dimension est fournie, l'autre sera calculée pour garder les proportions.
 */
int display_image(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int w, int h);

/**
 * Libère une texture de la mémoire.
 * @param texture La texture à libérer.
 */
void free_image(SDL_Texture* texture);

/**
 * Détruit la fenêtre et le renderer, et quitte SDL.
 * @param context Le SDL_Context à détruire.
 */
void destroy_context(SDL_Context context);

#endif // SDL_H