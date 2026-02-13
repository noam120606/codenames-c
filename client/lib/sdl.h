#ifndef SDL_H
#define SDL_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"

#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080

/**
 * Contexte SDL contenant la fenêtre et le renderer.
 * @param window Fenêtre SDL.
 * @param renderer Renderer SDL associé à la fenêtre.
 * @param clock Nombre de frame écoulé depuis le début de l'application.
 * @param fps Nombre de frames par seconde (calculé à partir du clock).
 * @param frame_start_time Timestamp du début de la frame actuelle (en ms).
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    long clock;
    float fps;
    Uint32 frame_start_time;
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
 * @param x La position x de l'image (`0` centre x)
 * @param y La position y de l'image (`0` centre y)
 * @param size_factor Facteur de taille (`1.0` taille originale)
 * @param angle Angle de rotation en degrés (0 par défaut, sens horaire)
 * @param flip Flags de flip: `SDL_FLIP_NONE` (défaut), `SDL_FLIP_HORIZONTAL`, `SDL_FLIP_VERTICAL`, ou combinaison
 * @param ratio Ratio de l'image (largeur/hauteur, `1` pour garder le ratio original)
 * @param opacity Opacité de l'image (0-255, 255 par défaut)
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur
 * Note : L'angle de rotation utilise un pivot au centre de l'image.
 */
int display_image(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, float size_factor, double angle, SDL_RendererFlip flip, float ratio, Uint8 opacity);
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