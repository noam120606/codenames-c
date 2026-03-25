#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "sdl.h"

#define INPUT_DEFAULT_MAX 256

/**
 * Identifiants des différents types d'input.
 */
typedef enum {
    INPUT_NONE = 0,
    INPUT_NAME,
    INPUT_JOIN_CODE,
    INPUT_HINT,
    INPUT_HINT_COUNT,
    INPUT_CHAT
} InputId;

/**
 * Configuration pour créer un `Input`.
 * Tous les champs ont des valeurs par défaut définies via `input_config_init`.
 * La structure peut être modifiée à tout moment et passée à `input_create`.
 * Note : Certains champs sont gérés en interne par l'input (text, cursor_pos, textures, etc.) et ne doivent pas être modifiés directement après la création de l'input. Seuls les champs "configurables" (x, y, w, h, font_path, etc.) peuvent être modifiés pour changer l'apparence ou le comportement de l'input.
 * @param x Offset horizontal relatif au centre de la fenêtre (pixels).
 *          Positif vers la droite, négatif vers la gauche.
 * @param y Offset vertical relatif au centre de la fenêtre (pixels).
 *          Positif vers le haut, négatif vers le bas.
 *          Si x = 0 et y = 0, le centre de l'input est au centre de la fenêtre.
 *          Formules appliquées dans input_create :
 *            screen_x = WIN_WIDTH/2  + x - w/2
 *            screen_y = WIN_HEIGHT/2 - y - h/2
 * @param w Largeur de l'input.
 * @param h Hauteur de l'input.
 * @param font_path Chemin vers la police à utiliser pour le texte de l'input.
 * @param font_size Taille de la police.
 * @param placeholders Tableau de chaînes de caractères utilisées comme placeholders cycliques lorsque l'input est vide. Peut être NULL si aucun placeholder n'est utilisé.
 * @param placeholder_count Nombre de placeholders dans le tableau `placeholders`.
 * @param submitted_label Texte à afficher avant le texte soumis (ex: "Pseudo : "). Peut être NULL pour ne rien afficher.
 * @param maxlen Longueur maximale du texte saisi. Si le texte dépasse cette longueur, il sera tronqué.
 * @param save_player_data Si 1, la valeur de l'input est sauvegardée dans datas/player.properties à chaque modification et rechargée au lancement du jeu. Utile pour des inputs comme le pseudo du joueur.
 * @param bg_color Couleur de fond de l'input (utilisée si `bg_path` est NULL).
 * @param border_color Couleur de la bordure de l'input.
 * @param text_color Couleur du texte saisi.
 * @param padding Padding intérieur de l'input (espace entre le bord et le texte).
 * @param centered Si 1, le texte/placeholder et le curseur sont centrés horizontalement. Si 0, ils sont alignés à gauche.
 * @param bg_path Chemin vers l'image de fond de l'input. Si NULL, un fond uni de couleur `bg_color` est utilisé. L'image est chargée automatiquement par `input_create` et doit être libérée lors de la destruction de l'input.
 * @param bg_padding Padding à appliquer avec l'image de fond (>= 0). Ignoré si `bg_path` est NULL. Si -1, le padding est calculé automatiquement pour centrer le contenu dans l'image de fond.
 * @param allowed_pattern Regex POSIX étendue appliquée à chaque caractère saisi. Si NULL, tout caractère est accepté. Ex: "^[A-Za-z0-9]$"
 * @param submit_pattern Regex POSIX étendue appliquée au texte complet au moment de la soumission. Si définie et non satisfaite, la soumission est ignorée. Ex: "^[0-9]{5}$"
 * @param init_text Texte initial à afficher dans l'input. Si NULL ou chaîne vide, l'input commence vide.
 * @param rect Rectangle SDL calculé automatiquement à partir de x/y/w/h, utilisé pour le rendu et la détection de clics.
 * @param text Buffer de texte saisi, géré en interne par l'input.
 * @param disabled Indique si la pression de la touche entrée est désactivée (1) ou non (0).
 * @param len Longueur actuelle du texte saisi.
 * @param cursor_pos Position actuelle du curseur dans le texte (index entre 0 et len).
 * @param focused Indique si l'input est actuellement focalisé (1) ou non (0).
 * @param submitted Indique si l'input a été soumis (1) ou non (0). Peut être utilisé pour déclencher des actions après la soumission.
 * @param bg_texture Texture SDL de l'image de fond, chargée automatiquement si `bg_path` est défini. Doit être libérée lors de la destruction de l'input.
 * @param sel_start Index de début de la sélection de texte (0 si aucune sélection).
 * @param sel_len Longueur de la sélection de texte (0 si aucune sélection).
 * @param sel_anchor Index d'ancrage de la sélection, utilisé pour étendre la sélection lors du déplacement du curseur avec Shift.
 * @param submitted_text Texte qui a été soumis lors de la dernière soumission. Peut être utilisé pour afficher le texte soumis ou pour d'autres logiques de jeu.
 * @param placeholder_index Index du placeholder actuellement affiché (si l'input est vide), utilisé pour faire défiler les placeholders.
 * @param placeholder_last_tick Timestamp du dernier changement de placeholder, utilisé pour faire défiler les placeholders à intervalles réguliers.
 * @param on_submit Fonction de rappel à appeler lorsque l'input est soumis (par exemple, en appuyant sur Entrée). La fonction doit prendre un `AppContext*` (contexte SDL) et un `const char*` (le texte soumis) et retourner void. Si NULL, aucune fonction ne sera appelée lors de la soumission.
 */
typedef struct InputConfig {
    int x;
    int y;
    int w;
    int h;
    const char* font_path;
    int font_size;
    const char** placeholders;
    int placeholder_count;
    const char* submitted_label;
    int maxlen;
    int save_player_data; /**< Si 1, sauvegarde/charge la valeur de l'input dans datas/player.properties. */
    SDL_Color bg_color;
    SDL_Color border_color;
    SDL_Color text_color;
    int padding;
    int centered;          /**< Si 1, le texte/placeholder et le curseur sont centrés horizontalement. */
    const char* bg_path;   /**< Chemin vers l'image de fond (NULL = pas d'image, fond uni). Chargée automatiquement par input_create. */
    int bg_padding;        /**< Padding à appliquer avec l'image de fond (>= 0). Ignoré si bg_path est NULL. -1 = auto. */
    const char* allowed_pattern; /**< Regex POSIX étendue appliquée à chaque caractère saisi. NULL = tout accepter. Ex: "^[A-Za-z0-9]$" */
    const char* submit_pattern;  /**< Regex POSIX étendue appliquée au texte complet au submit. NULL = pas de validation de soumission. Ex: "^[0-9]{5}$" */
    const char* init_text;
    SDL_Rect rect;
    char* text;
    int disabled;
    int len;
    int cursor_pos;
    int focused;
    int submitted;
    SDL_Texture* bg_texture;
    int sel_start;
    int sel_len;
    int sel_anchor;
    char* submitted_text;
    int placeholder_index;
    Uint32 placeholder_last_tick;
    void (*on_submit)(AppContext*, const char*);
} InputConfig;

/**
 * Structure d'input.
 */
typedef struct Input {
    InputId id;
    InputConfig* cfg;
} Input;

/**
 * Clés de configuration modifiables via `edit_in_cfg`.
 */
typedef enum InputCfgKey {
    IN_CFG_X = 100,
    IN_CFG_Y,
    IN_CFG_W,
    IN_CFG_H,
    IN_CFG_FONT_PATH,
    IN_CFG_FONT_SIZE,
    IN_CFG_PLACEHOLDERS,
    IN_CFG_PLACEHOLDER_COUNT,
    IN_CFG_SUBMITTED_LABEL,
    IN_CFG_MAXLEN,
    IN_CFG_SAVE_PLAYER_DATA,
    IN_CFG_BG_COLOR,
    IN_CFG_BORDER_COLOR,
    IN_CFG_TEXT_COLOR,
    IN_CFG_PADDING,
    IN_CFG_CENTERED,
    IN_CFG_BG_PATH,
    IN_CFG_BG_PADDING,
    IN_CFG_ALLOWED_PATTERN,
    IN_CFG_SUBMIT_PATTERN,
    IN_CFG_INIT_TEXT,
    IN_CFG_RECT,
    IN_CFG_TEXT,
    IN_CFG_LEN,
    IN_CFG_CURSOR_POS,
    IN_CFG_FOCUSED,
    IN_CFG_SUBMITTED,
    IN_CFG_BG_TEXTURE,
    IN_CFG_SEL_START,
    IN_CFG_SEL_LEN,
    IN_CFG_SEL_ANCHOR,
    IN_CFG_SUBMITTED_TEXT,
    IN_CFG_PLACEHOLDER_INDEX,
    IN_CFG_PLACEHOLDER_LAST_TICK,
    IN_CFG_ON_SUBMIT,
} InputCfgKey;



/** Initialise une `InputConfig` avec des valeurs par défaut. */
InputConfig* input_config_init();

/**
 * Crée un nouvel input à partir d'une configuration.
 * Si `cfg` est NULL, les valeurs par défaut sont utilisées.
 * L'input créé doit être détruit avec `input_destroy` pour éviter les fuites de mémoire.
 * @param renderer Renderer SDL utilisé pour charger la texture de fond si `bg_path` est défini dans la configuration.
 * @param id Identifiant de l'input (ex: INPUT_ID_NAME). Peut être utilisé pour différencier les inputs dans les callbacks.
 * @param cfg_in Pointeur vers la configuration de l'input. Si NULL, les valeurs par défaut sont utilisées. La configuration est copiée, donc le pointeur peut être libéré ou modifié après l'appel sans affecter l'input créé.
 * @return Pointeur vers l'input créé, ou NULL en cas d'erreur (par exemple, échec d'allocation mémoire ou de chargement de la texture de fond).
 */
Input* input_create(SDL_Renderer* renderer, InputId id, const InputConfig* cfg);

/** Détruit un input. 
 * Libère la mémoire associée à l'input et ses ressources (texte, texture de fond).
 * @param in Pointeur vers l'input à détruire. Si NULL, la fonction ne fait rien.
 */
void input_destroy(Input* in);

/**
 * Gère les événements SDL pour l'input (clics, saisie de texte, etc.).
 * Doit être appelé à chaque événement SDL dans la boucle principale.
 * @param context Contexte SDL, passé aux callbacks de soumission.
 * @param in Pointeur vers l'input à gérer.
 * @param e Pointeur vers l'événement SDL à traiter.
 */
void input_handle_event(AppContext* context, Input* in, SDL_Event* e);

/** Affiche l'input sur le renderer.
 * Doit être appelé à chaque frame dans la boucle de rendu.
 * @param renderer Le renderer SDL sur lequel dessiner l'input.
 * @param in Pointeur vers l'input à afficher.
 */
void input_render(SDL_Renderer* renderer, Input* in);

/**
 * Récupère le texte actuel de l'input.
 * @param in Pointeur vers l'input.
 * @return Le texte actuel de l'input, ou NULL si l'input est NULL.
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
 * Soumet le texte courant de l'input (même comportement qu'appuyer sur Entrée).
 * Marque l'input comme soumis, appelle le callback `on_submit` si défini,
 * puis vide le texte courant de l'input.
 * @param context Contexte SDL passé au callback de soumission.
 * @param in Pointeur vers l'input.
 */
void input_submit(AppContext* context, Input* in);

/**
 * Définit le texte de l'input. Si le texte dépasse la longueur maximale, il sera tronqué.
 * @param in Pointeur vers l'input.
 * @param text Nouveau texte à définir.
 */
void input_set_text(Input* in, const char* text);

/**
 * Définit une fonction de rappel à appeler lorsque l'input est soumis (par exemple, en appuyant sur Entrée).
 * @param in Pointeur vers l'input.
 * @param cb Pointeur vers la fonction de rappel à appeler lors de la soumission. La fonction doit prendre un `AppContext*` (contexte SDL) et un `const char*` (le texte soumis) et retourner void. Si NULL, aucune fonction ne sera appelée.
 */
void input_set_on_submit(Input* in, void (*cb)(AppContext*, const char*));

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

/**
 * Modifie une valeur de configuration d'un input identifié par son id.
 * @param id    L'ID de l'input à modifier.
 * @param key   Champ ciblé dans la configuration.
 * @param value Valeur à appliquer (entier direct pour champs int/bool, pointeur casté en `intptr_t` pour les champs pointeurs, et adresse d'une structure pour `IN_CFG_BG_COLOR` / `IN_CFG_BORDER_COLOR` / `IN_CFG_TEXT_COLOR` / `IN_CFG_RECT`).
 * @return EXIT_SUCCESS si l'input existe, sinon EXIT_FAILURE.
 */
int edit_in_cfg(InputId id, InputCfgKey key, intptr_t value);

#endif /* INPUT_H */
