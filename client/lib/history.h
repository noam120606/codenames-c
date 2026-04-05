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
} History;

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
 * Construit des lignes prêtes à afficher pour un historique d'équipe.
 * @param history Historique source.
 * @param lines Tampon de sortie de type [max_lines][HISTORY_LINE_SIZE].
 * @param max_lines Nombre de lignes disponibles dans le tampon.
 * @return Nombre de lignes réellement écrites.
 */
int history_build_lines(const History* history, char lines[][HISTORY_LINE_SIZE], int max_lines);

#endif /* HISTORY_H */
