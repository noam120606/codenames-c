/**
 * @file history.h
 * @brief Outils de gestion de l'historique des tours côté client.
 */

#ifndef HISTORY_H
#define HISTORY_H

#include "../lib/game.h"

typedef struct AppContext AppContext;

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
 */
void history_start_turn(AppContext* context, Team team, const char* hint);

/**
 * Garantit qu'au moins un tour existe pour l'équipe.
 * Si aucun tour n'existe, en crée un avec l'indice fourni.
 * @param context Contexte applicatif.
 * @param team Équipe concernée.
 * @param hint Indice à associer au tour créé (peut être NULL).
 */
void history_ensure_turn(AppContext* context, Team team, const char* hint);

/**
 * Ajoute un mot révélé au dernier tour de l'équipe.
 * Si aucun tour n'existe, un tour est créé automatiquement.
 * @param context Contexte applicatif.
 * @param team Équipe concernée.
 * @param word Mot révélé à enregistrer.
 */
void history_append_revealed_word(AppContext* context, Team team, const char* word);

#endif /* HISTORY_H */
