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

static void history_touch(History* history) {
    if (!history) return;
    history->revision++;
}

static int history_push_line(char lines[][HISTORY_LINE_SIZE], int max_lines, int current_count, const char* line) {
    if (!lines || max_lines <= 0 || current_count < 0 || !line) return current_count;
    if (current_count >= max_lines) return current_count;

    strncpy(lines[current_count], line, HISTORY_LINE_SIZE - 1);
    lines[current_count][HISTORY_LINE_SIZE - 1] = '\0';
    return current_count + 1;
}

static int history_append_wrapped_line(const char* line, TTF_Font* font, int max_text_width, char out_lines[][HISTORY_LINE_SIZE], int max_lines, int current_count) {
    if (!out_lines || max_lines <= 0 || current_count < 0 || current_count >= max_lines) return current_count;

    const char* cursor = (line && line[0] != '\0') ? line : " ";

    while (cursor[0] != '\0' && current_count < max_lines) {
        if (!font) {
            strncpy(out_lines[current_count], cursor, HISTORY_LINE_SIZE - 1);
            out_lines[current_count][HISTORY_LINE_SIZE - 1] = '\0';
            current_count++;
            break;
        }

        int full_width = 0;
        if (TTF_SizeUTF8(font, cursor, &full_width, NULL) != 0 || full_width <= max_text_width) {
            strncpy(out_lines[current_count], cursor, HISTORY_LINE_SIZE - 1);
            out_lines[current_count][HISTORY_LINE_SIZE - 1] = '\0';
            current_count++;
            break;
        }

        int remaining_len = (int)strlen(cursor);
        int best_fit_len = 0;
        int best_space_break = -1;

        for (int i_char = 0; i_char < remaining_len;) {
            unsigned char c = (unsigned char)cursor[i_char];
            int char_len = 1;
            if ((c & 0x80) == 0x00) char_len = 1;
            else if ((c & 0xE0) == 0xC0) char_len = 2;
            else if ((c & 0xF0) == 0xE0) char_len = 3;
            else if ((c & 0xF8) == 0xF0) char_len = 4;

            if (i_char + char_len > remaining_len) {
                char_len = 1;
            }

            int candidate_len = i_char + char_len;
            if (candidate_len >= HISTORY_LINE_SIZE) {
                candidate_len = HISTORY_LINE_SIZE - 1;
            }

            char probe[HISTORY_LINE_SIZE];
            memcpy(probe, cursor, candidate_len);
            probe[candidate_len] = '\0';

            int probe_width = 0;
            if (TTF_SizeUTF8(font, probe, &probe_width, NULL) != 0 || probe_width > max_text_width) {
                break;
            }

            best_fit_len = candidate_len;
            if (cursor[i_char] == ' ') {
                best_space_break = i_char;
            }

            if (candidate_len >= HISTORY_LINE_SIZE - 1) {
                break;
            }

            i_char += char_len;
        }

        int split_at = (best_space_break > 0) ? best_space_break : best_fit_len;
        if (split_at <= 0) {
            unsigned char first = (unsigned char)cursor[0];
            if ((first & 0x80) == 0x00) split_at = 1;
            else if ((first & 0xE0) == 0xC0) split_at = 2;
            else if ((first & 0xF0) == 0xE0) split_at = 3;
            else if ((first & 0xF8) == 0xF0) split_at = 4;
            else split_at = 1;
        }

        if (split_at > remaining_len) split_at = remaining_len;

        int write_len = split_at;
        while (write_len > 0 && cursor[write_len - 1] == ' ') {
            write_len--;
        }
        if (write_len <= 0) {
            write_len = split_at;
        }
        if (write_len >= HISTORY_LINE_SIZE) {
            write_len = HISTORY_LINE_SIZE - 1;
        }

        memcpy(out_lines[current_count], cursor, write_len);
        out_lines[current_count][write_len] = '\0';
        current_count++;

        cursor += split_at;
        while (cursor[0] == ' ') {
            cursor++;
        }
    }

    return current_count;
}

static int history_font_path_equals(const char* a, const char* b) {
    const char* left = a ? a : "";
    const char* right = b ? b : "";
    return strcmp(left, right) == 0;
}

static void history_set_font_path(char* dst, int dst_size, const char* src) {
    if (!dst || dst_size <= 0) return;

    const char* value = src ? src : "";
    strncpy(dst, value, (size_t)dst_size - 1);
    dst[dst_size - 1] = '\0';
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
    history->revision = 1;
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
    history_touch(history);
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
            history_touch(history);
            return;
        }
    }
}

int history_update_last_turn(History* history, const char* spy_name, const char* hint, int hint_count) {
    if (!history || history->turn_count <= 0) return EXIT_FAILURE;

    Turn* turn = &history->turns[history->turn_count - 1];

    if (spy_name) {
        strncpy(turn->spy_name, spy_name, sizeof(turn->spy_name) - 1);
        turn->spy_name[sizeof(turn->spy_name) - 1] = '\0';
    }

    if (hint) {
        strncpy(turn->hint, hint, sizeof(turn->hint) - 1);
        turn->hint[sizeof(turn->hint) - 1] = '\0';
    }

    if (hint_count >= 0) {
        turn->hint_count = hint_count;
    }

    history_touch(history);
    return EXIT_SUCCESS;
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

int history_build_wrapped_lines(const History* history, const char* font_path, int font_size, int max_text_width, char lines[][HISTORY_LINE_SIZE], int max_lines) {
    if (!lines || max_lines <= 0) return 0;

    if (max_text_width < 1) {
        max_text_width = 1;
    }

    char raw_lines[HISTORY_MAX_BASE_LINES][HISTORY_LINE_SIZE] = {{0}};
    int raw_line_count = history_build_lines(history, raw_lines, HISTORY_MAX_BASE_LINES);

    TTF_Font* history_font = NULL;
    if (font_path && font_path[0] != '\0' && font_size > 0) {
        history_font = TTF_OpenFont(font_path, font_size);
    }

    int total_lines = 0;
    for (int i_line = 0; i_line < raw_line_count && total_lines < max_lines; i_line++) {
        total_lines = history_append_wrapped_line(
            raw_lines[i_line],
            history_font,
            max_text_width,
            lines,
            max_lines,
            total_lines
        );
    }

    if (history_font) {
        TTF_CloseFont(history_font);
    }

    if (total_lines <= 0) {
        lines[0][0] = '\0';
        total_lines = 1;
    }

    return total_lines;
}

void history_wrap_cache_invalidate(HistoryWrapCache* cache) {
    if (!cache) return;
    memset(cache, 0, sizeof(HistoryWrapCache));
}

int history_build_wrapped_lines_cached(const History* history, const char* font_path, int font_size, int max_text_width, HistoryWrapCache* cache) {
    if (!cache) return 0;

    if (max_text_width < 1) {
        max_text_width = 1;
    }

    unsigned int source_revision = history ? history->revision : 0;

    int cache_hit =
        cache->is_valid &&
        cache->source_history == history &&
        cache->source_revision == source_revision &&
        cache->max_text_width == max_text_width &&
        cache->font_size == font_size &&
        history_font_path_equals(cache->font_path, font_path);

    if (cache_hit) {
        return cache->total_lines;
    }

    int total_lines = history_build_wrapped_lines(
        history,
        font_path,
        font_size,
        max_text_width,
        cache->lines,
        HISTORY_MAX_RENDER_LINES
    );

    cache->source_history = history;
    cache->source_revision = source_revision;
    cache->max_text_width = max_text_width;
    cache->font_size = font_size;
    history_set_font_path(cache->font_path, HISTORY_FONT_PATH_MAX, font_path);
    cache->total_lines = total_lines;
    cache->is_valid = 1;

    return total_lines;
}
