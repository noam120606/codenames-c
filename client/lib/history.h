/**
 * @file history.h
 * @brief Outils de gestion de l'historique des tours côté client.
 */

#ifndef HISTORY_H
#define HISTORY_H

#include "../lib/game.h"

typedef struct AppContext AppContext;

/** Taille maximale (en octets) d'une ligne d'historique formatée. */
#define HISTORY_LINE_SIZE 128

/**
 * Nombre maximal de lignes logiques produites par history_build_lines.
 * Une ligne logique peut ensuite être découpée en plusieurs lignes d'affichage.
 */
#define HISTORY_MAX_BASE_LINES ((NB_WORDS * 2) + 4)

/** Nombre maximal de lignes d'affichage après wrapping. */
#define HISTORY_MAX_RENDER_LINES (HISTORY_MAX_BASE_LINES * 4)

/** Taille maximale stockée pour le chemin de police dans le cache de rendu. */
#define HISTORY_FONT_PATH_MAX 260

/**
 * Représente un tour de jeu pour une équipe.
 * @param spy_name Nom du joueur qui a soumis l'indice.
 * @param hint_count Nombre de mots associés à l'indice (0 si indisponible).
 * @param agent_name Nom du joueur qui a révélé le(s) mot(s) du tour (vide si inconnu).
 * @param hint Indice soumis par l'espion (vide si indisponible).
 * @param revealed_words Mots révélés pendant ce tour (tableau de taille NB_WORDS).
 */
typedef struct Turn {
	char spy_name[32];
	int hint_count;
	char agent_name[32];
	char hint[64];
	char revealed_words[NB_WORDS][64];
} Turn;

/**
 * Historique des tours pour une équipe.
 * @param turns Tableau des tours enregistrés.
 * @param turn_count Nombre de tours actuellement stockés.
 */
typedef struct History {
	Turn turns[NB_WORDS];
	int turn_count;
	unsigned int revision;
} History;

/**
 * Cache de lignes d'historique déjà découpées pour l'affichage.
 * Recalculé uniquement quand les données ou les paramètres de rendu changent.
 */
typedef struct HistoryWrapCache {
	const History* source_history;
	unsigned int source_revision;
	int max_text_width;
	int font_size;
	char font_path[HISTORY_FONT_PATH_MAX];
	int total_lines;
	int is_valid;
	char lines[HISTORY_MAX_RENDER_LINES][HISTORY_LINE_SIZE];
} HistoryWrapCache;

/**
 * Retourne l'historique associé à une équipe.
 * @param game Partie cible.
 * @param team Équipe cible (TEAM_RED ou TEAM_BLUE).
 * @return Pointeur vers l'historique de l'équipe, ou NULL si invalide.
 */
History* history_get_for_team(Game* game, Team team);

/**
 * Déduit l'équipe active à partir d'un état de tour agent.
 * @param state État de jeu courant.
 * @return TEAM_RED ou TEAM_BLUE pour les états agent, TEAM_NONE sinon.
 */
Team history_team_from_agent_state(GameState state);

/**
 * Réinitialise un historique d'équipe.
 * @param history Historique à vider.
 */
void history_reset(History* history);

/**
 * Crée un nouveau tour dans l'historique de l'équipe.
 * Le nom d'espion est résolu depuis le lobby courant si possible.
 * @param context Contexte applicatif.
 * @param team Équipe concernée.
 * @param hint Indice associé au tour (peut être NULL).
 * @param hint_count Nombre associé à l'indice (0 si indisponible).
 */
void history_start_turn(AppContext* context, Team team, const char* hint, int hint_count);

/**
 * Garantit qu'au moins un tour existe pour l'équipe.
 * Si aucun tour n'existe, en crée un avec l'indice fourni.
 * @param context Contexte applicatif.
 * @param team Équipe concernée.
 * @param hint Indice à associer au tour créé (peut être NULL).
 * @param hint_count Nombre associé à l'indice (0 si indisponible).
 */
void history_ensure_turn(AppContext* context, Team team, const char* hint, int hint_count);

/**
 * Ajoute un mot révélé au dernier tour de l'équipe.
 * Si aucun tour n'existe, un tour est créé automatiquement.
 * @param context Contexte applicatif.
 * @param team Équipe concernée.
 * @param word Mot révélé à enregistrer.
 */
void history_append_revealed_word(AppContext* context, Team team, const char* word);

/**
 * Met à jour les métadonnées du dernier tour (nom espion/indice/compte) et invalide le cache associé.
 * @param history Historique à modifier.
 * @param spy_name Nom de l'espion (peut être NULL pour ne pas modifier).
 * @param hint Indice (peut être NULL pour ne pas modifier).
 * @param hint_count Nombre associé à l'indice (valeur >= 0 pour modifier).
 * @return EXIT_SUCCESS si le dernier tour a été mis à jour, EXIT_FAILURE sinon.
 */
int history_update_last_turn(History* history, const char* spy_name, const char* hint, int hint_count);

/**
 * Construit des lignes prêtes à afficher pour un historique d'équipe.
 * @param history Historique source.
 * @param lines Tampon de sortie de type [max_lines][HISTORY_LINE_SIZE].
 * @param max_lines Nombre de lignes disponibles dans le tampon.
 * @return Nombre de lignes réellement écrites.
 */
int history_build_lines(const History* history, char lines[][HISTORY_LINE_SIZE], int max_lines);

/**
 * Construit des lignes prêtes à afficher avec retour à la ligne automatique.
 * Le wrapping est calculé en fonction d'une largeur maximale en pixels.
 * Si la police est indisponible, les lignes sont copiées sans mesure de largeur.
 * @param history Historique source.
 * @param font_path Chemin de la police utilisée pour mesurer la largeur (peut être NULL).
 * @param font_size Taille de police utilisée pour la mesure (doit être > 0 avec font_path).
 * @param max_text_width Largeur maximale d'une ligne en pixels.
 * @param lines Tampon de sortie de type [max_lines][HISTORY_LINE_SIZE].
 * @param max_lines Nombre de lignes disponibles dans le tampon.
 * @return Nombre de lignes réellement écrites.
 */
int history_build_wrapped_lines(const History* history, const char* font_path, int font_size, int max_text_width, char lines[][HISTORY_LINE_SIZE], int max_lines);

/**
 * Invalide un cache de wrapping historique.
 * @param cache Cache à invalider.
 */
void history_wrap_cache_invalidate(HistoryWrapCache* cache);

/**
 * Récupère les lignes d'historique wrapées via un cache.
 * Le wrapping est recalculé uniquement si nécessaire.
 * @param history Historique source.
 * @param font_path Chemin de la police utilisée pour mesurer la largeur (peut être NULL).
 * @param font_size Taille de police utilisée pour la mesure.
 * @param max_text_width Largeur maximale d'une ligne en pixels.
 * @param cache Cache de rendu à remplir/réutiliser.
 * @return Nombre total de lignes disponibles dans cache->lines.
 */
int history_build_wrapped_lines_cached(const History* history, const char* font_path, int font_size, int max_text_width, HistoryWrapCache* cache);

#endif /* HISTORY_H */
