/**
 * @file sdl.h
 * @brief Initialisation SDL et contexte principal de l'application.
 */

#ifndef SDL_H
#define SDL_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../SDL2/include/SDL2/SDL_mixer.h"

#include "../lib/game.h"
#include "../lib/user.h"
#include "../lib/list.h"
#include "../lib/lobby.h"

#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080
#define WIN_MIN_WIDTH 960
#define WIN_MIN_HEIGHT 540

typedef struct Card Card;

/**
 * Contexte SDL contenant la fenêtre et le renderer.
 * @param window Fenêtre SDL.
 * @param renderer Renderer SDL associé à la fenêtre.
 * @param bg_color Couleur de fond de la fenêtre.
 * @param clock Nombre de frame écoulé depuis le début de l'application.
 * @param fps Nombre de frames par seconde (calculé à partir du clock).
 * @param sock Identifiant socket.
 * @param lobby Pointeur vers la structure de lobby.
 * @param app_state État actuel du jeu (menu, lobby, partie, etc.).
 * @param player_role Rôle du joueur (ROLE_AGENT ou ROLE_SPY).
 * @param player_team Équipe du joueur (TEAM_RED, TEAM_BLUE, TEAM_NONE).
 * @param frame_start_time Timestamp du début de la frame actuelle (en ms).
 * @param ping_ms Ping TCP courant en millisecondes (-1 si indisponible).
 * @param player_id Identifiant numerique attribue par le serveur (-1 si inconnu).
 * @param music_volume Volume de la musique (0-128).
 * @param sound_effects_volume Volume des effets sonores (0-128).
 * @param windowed_width Largeur de la fenêtre en mode fenêtré (pour restaurer après un plein écran).
 * @param windowed_height Hauteur de la fenêtre en mode fenêtré (pour restaurer après un plein écran).
 * @param windowed_x Position x de la fenêtre en mode fenêtré (pour restaurer après un plein écran).
 * @param windowed_y Position y de la fenêtre en mode fenêtré (pour restaurer après un plein écran).
 * @param last_fullscreen_toggle_ms Timestamp du dernier basculement plein écran (en ms, pour éviter les basculements rapides accidentels).
 * @param player_uuid UUID unique du joueur (persisté dans datas/uuid).
 * @param player_name Nom du joueur (persisté dans datas/name).
 * @param version Version du jeu (chargée depuis VERSION).
 */
typedef struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Color bg_color;
    long clock;
    float fps;
    int sock;
    Lobby* lobby;
    AppState app_state;
    UserRole player_role; 
    Team player_team; 
    Uint32 frame_start_time;
    int ping_ms;
    int player_id;
    int music_volume;
    int sound_effects_volume;
    int windowed_width;
    int windowed_height;
    int windowed_x;
    int windowed_y;
    Uint32 last_fullscreen_toggle_ms;
    char* player_uuid;
    char* player_name;
    char version[16];
} AppContext;

/**
 * Initialise SDL et crée une fenêtre et un renderer.
 * @return Un AppContext contenant la fenêtre et le renderer,
 *         ou un AppContext avec des pointeurs `NULL` en cas d'erreur.
 */
AppContext init_sdl();

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
 * Bascule le mode plein écran / fenêtré de la fenêtre.
 * @param context Le AppContext contenant la fenêtre.
 */
void toggle_fullscreen(AppContext* context);

/**
 * Détruit la fenêtre et le renderer, et quitte SDL.
 * @param context Le AppContext à détruire.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE en cas d'erreur.
 */
int destroy_context(AppContext* context);

#endif // SDL_H