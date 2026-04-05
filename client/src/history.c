#include "../lib/all.h"

/**
 * @file history.c
 * @brief Implémentation de la gestion d'historique des tours côté client.
 */

static const char* history_fallback_team_name(Team team) {
    if (team == TEAM_RED) return "Equipe rouge";
    if (team == TEAM_BLUE) return "Equipe bleue";
    return "Equipe";
}

static const char* history_find_team_spy_name(const AppContext* context, Team team) {
    if (!context || !context->lobby) return NULL;

    for (int i = 0; i < MAX_USERS; i++) {
        User* user = context->lobby->users[i];
        if (!user || !user->name) continue;
        if (user->team == team && user->role == ROLE_SPY) {
            return user->name;
        }
    }

    return NULL;
}

History* history_get_for_team(Game* game, Team team) {
    if (!game) return NULL;
    if (team == TEAM_RED) return &game->red_history;
    if (team == TEAM_BLUE) return &game->blue_history;
    return NULL;
}

Team history_team_from_agent_state(GameState state) {
    if (state == GAMESTATE_TURN_RED_AGENT) return TEAM_RED;
    if (state == GAMESTATE_TURN_BLUE_AGENT) return TEAM_BLUE;
    return TEAM_NONE;
}

void history_reset(History* history) {
    if (!history) return;
    memset(history, 0, sizeof(History));
}

void history_start_turn(AppContext* context, Team team, const char* hint) {
    if (!context || !context->lobby || !context->lobby->game) return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;
    if (history->turn_count >= NB_WORDS) return;

    Turn* turn = &history->turns[history->turn_count];
    memset(turn, 0, sizeof(Turn));

    const char* spy_name = history_find_team_spy_name(context, team);
    if (!spy_name || spy_name[0] == '\0') {
        spy_name = history_fallback_team_name(team);
    }

    strncpy(turn->spy_name, spy_name, sizeof(turn->spy_name) - 1);
    turn->spy_name[sizeof(turn->spy_name) - 1] = '\0';

    if (hint && hint[0] != '\0') {
        strncpy(turn->hint, hint, sizeof(turn->hint) - 1);
        turn->hint[sizeof(turn->hint) - 1] = '\0';
    }

    history->turn_count++;
}

void history_ensure_turn(AppContext* context, Team team, const char* hint) {
    if (!context || !context->lobby || !context->lobby->game) return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;
    if (history->turn_count > 0) return;

    history_start_turn(context, team, hint);
}

void history_append_revealed_word(AppContext* context, Team team, const char* word) {
    if (!context || !context->lobby || !context->lobby->game) return;
    if (!word || word[0] == '\0') return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;

    if (history->turn_count <= 0) {
        history_start_turn(context, team, context->lobby->game->current_hint);
    }

    if (history->turn_count <= 0) return;

    Turn* turn = &history->turns[history->turn_count - 1];
    for (int i = 0; i < NB_WORDS; i++) {
        if (turn->revealed_words[i][0] == '\0') {
            strncpy(turn->revealed_words[i], word, sizeof(turn->revealed_words[i]) - 1);
            turn->revealed_words[i][sizeof(turn->revealed_words[i]) - 1] = '\0';
            return;
        }
    }
}
