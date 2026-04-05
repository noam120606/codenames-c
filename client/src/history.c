#include "../lib/all.h"

/**
 * @file history.c
 * @brief Implémentation de la gestion d'historique des tours côté client.
 */

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

static int history_push_line(char lines[][HISTORY_LINE_SIZE], int max_lines, int current_count, const char* line) {
    if (!lines || max_lines <= 0 || current_count < 0 || !line) return current_count;
    if (current_count >= max_lines) return current_count;

    strncpy(lines[current_count], line, HISTORY_LINE_SIZE - 1);
    lines[current_count][HISTORY_LINE_SIZE - 1] = '\0';
    return current_count + 1;
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

void history_start_turn(AppContext* context, Team team, const char* hint, int hint_count) {
    if (!context || !context->lobby || !context->lobby->game) return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;
    if (history->turn_count >= NB_WORDS) return;

    Turn* turn = &history->turns[history->turn_count];
    memset(turn, 0, sizeof(Turn));

    const char* spy_name = history_find_team_spy_name(context, team);
    if (!spy_name || spy_name[0] == '\0') {
        spy_name = "Vous avez";
    }

    strncpy(turn->spy_name, spy_name, sizeof(turn->spy_name) - 1);
    turn->spy_name[sizeof(turn->spy_name) - 1] = '\0';

    if (hint && hint[0] != '\0') {
        strncpy(turn->hint, hint, sizeof(turn->hint) - 1);
        turn->hint[sizeof(turn->hint) - 1] = '\0';
    }

    turn->hint_count = (hint_count > 0) ? hint_count : 0;

    history->turn_count++;
}

void history_ensure_turn(AppContext* context, Team team, const char* hint, int hint_count) {
    if (!context || !context->lobby || !context->lobby->game) return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;
    if (history->turn_count > 0) return;

    history_start_turn(context, team, hint, hint_count);
}

void history_append_revealed_word(AppContext* context, Team team, const char* word) {
    if (!context || !context->lobby || !context->lobby->game) return;
    if (!word || word[0] == '\0') return;

    History* history = history_get_for_team(context->lobby->game, team);
    if (!history) return;

    if (history->turn_count <= 0) {
        history_start_turn(
            context,
            team,
            context->lobby->game->current_hint,
            context->lobby->game->current_hint_count
        );
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

int history_build_lines(const History* history, char lines[][HISTORY_LINE_SIZE], int max_lines) {
    if (!lines || max_lines <= 0) return 0;

    int line_count = 0;
    if (!history || history->turn_count <= 0) {
        line_count = history_push_line(lines, max_lines, line_count, "Aucun tour joué pour le moment.");
        return line_count;
    }

    for (int i_turn = 0; i_turn < history->turn_count; i_turn++) {
        const Turn* turn = &history->turns[i_turn];
        const char* spy_name = (turn->spy_name[0] != '\0') ? turn->spy_name : "Equipe";

        char header[HISTORY_LINE_SIZE];
        format_to(
            header,
            sizeof(header),
            "Tour %d - %s a soumis %s en %d",
            i_turn + 1,
            spy_name,
            (turn->hint[0] != '\0') ? turn->hint : "???",
            turn->hint_count
        );
        line_count = history_push_line(lines, max_lines, line_count, header);

        int words_added = 0;
        for (int i_word = 0; i_word < NB_WORDS; i_word++) {
            if (turn->revealed_words[i_word][0] == '\0') continue;

            char word_line[HISTORY_LINE_SIZE];
            format_to(word_line, sizeof(word_line), "  - %s", turn->revealed_words[i_word]);
            line_count = history_push_line(lines, max_lines, line_count, word_line);
            words_added++;
        }

        if (words_added == 0) {
            line_count = history_push_line(lines, max_lines, line_count, "  - Aucun mot revele");
        }
    }

    return line_count;
}
