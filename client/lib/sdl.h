#ifndef SDL_H
#define SDL_H

#include "../SDL2/include/SDL2/SDL.h"
#include "../SDL2/include/SDL2/SDL_image.h"
#include "../SDL2/include/SDL2/SDL_ttf.h"
#include "../SDL2/include/SDL2/SDL_mixer.h"

#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080

/**
 * États possibles du jeu.
 * @param GAME_STATE_MENU État du menu principal.
 * @param GAME_STATE_LOBBY État du lobby (attente des joueurs).
 * @param GAME_STATE_PLAYING État de la partie en cours.
 */
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_LOBBY,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED
} GameState;

/** Rôles possibles d'un utilisateur.
 * @param ROLE_ESPION donne des indices.
 * @param ROLE_AGENT devine les mots.
 */
typedef enum UserRole {
    ROLE_SPY,
    ROLE_AGENT
} UserRole;

/** 
 * Catégories de mots dans la grille de Codenames.
 * Les mots sont classés en 4 catégories :
 * @param TEAM_NEUTRAL mot neutre (aucune équipe).
 * @param TEAM_RED mot appartenant à l'équipe rouge.
 * @param TEAM_BLUE mot appartenant à l'équipe bleue.
 * @param TEAM_BLACK mot assassin (met fin à la partie si révélé).
 */
typedef enum Team {
    TEAM_NEUTRAL,
    TEAM_RED,
    TEAM_BLUE,
    TEAM_BLACK,
} Team;

typedef struct Card Card;

/**
 * Contexte SDL contenant la fenêtre et le renderer.
 * @param window Fenêtre SDL.
 * @param renderer Renderer SDL associé à la fenêtre.
 * @param clock Nombre de frame écoulé depuis le début de l'application.
 * @param fps Nombre de frames par seconde (calculé à partir du clock).
 * @param sock Identifiant socket.  
 * @param lobby_id Identifiant du lobby auquel le client est connecté (-1 si aucun).
 * @param lobby_code Code du lobby auquel le client est connecté (NULL si aucun).
 * @param game_state État actuel du jeu (menu, lobby, partie, etc.).
 * @param player_role Rôle du joueur (ROLE_AGENT ou ROLE_SPY).
 * @param player_team Équipe du joueur (TEAM_RED, TEAM_BLUE, TEAM_NEUTRAL).
 * @param frame_start_time Timestamp du début de la frame actuelle (en ms).
 */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    long clock;
    float fps;
    int sock;
    int lobby_id;
    char* lobby_code;
    GameState game_state;
    UserRole player_role; 
    Team player_team; 
    Uint32 frame_start_time;
    int music_volume;          /**< Volume de la musique (0-128). */
    int sound_effects_volume;  /**< Volume des effets sonores (0-128). */
    Card* cards[25];
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