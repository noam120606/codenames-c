#include "../lib/all.h"

static Text* chat_colored_prefix_text = NULL;

static const SDL_Color CHAT_TEAM_BLUE_COLOR = {50, 80, 150, 255};
static const SDL_Color CHAT_TEAM_RED_COLOR = {150, 50, 50, 255};

static const int CHAT_LINE_GAP = 15;
static const int CHAT_LEFT_PADDING = 8;
static const int CHAT_RIGHT_PADDING = 8;
static const int CHAT_BOTTOM_LINE_Y = -42;

static int chat_font_path_equals(const char* a, const char* b) {
	const char* left = a ? a : "";
	const char* right = b ? b : "";
	return strcmp(left, right) == 0;
}

static int chat_text_width(const Text* text) {
	int width = 0;
	if (text && text->texture) {
		SDL_QueryTexture(text->texture, NULL, NULL, &width, NULL);
	}
	return width;
}

static int chat_left_anchored_rel_x(const Window* chat_window, int text_width) {
	if (!chat_window || !chat_window->cfg) return 0;
	return -(chat_window->cfg->w / 2) + CHAT_LEFT_PADDING + (text_width / 2);
}

static int chat_left_anchored_rel_x_with_offset(const Window* chat_window, int text_width, int left_offset) {
	return chat_left_anchored_rel_x(chat_window, text_width) + left_offset;
}

static void free_chat_message(void* data) {
	free(data);
}

static Team chat_find_team_by_player_name(const AppContext* context, const char* player_name) {
	if (!context || !player_name || player_name[0] == '\0') return TEAM_NONE;

	if (context->player_name && strcmp(context->player_name, player_name) == 0) {
		return context->player_team;
	}

	if (!context->lobby) return TEAM_NONE;

	for (int i = 0; i < MAX_USERS; i++) {
		User* user = context->lobby->users[i];
		if (!user || !user->name) continue;
		if (strcmp(user->name, player_name) == 0) {
			return user->team;
		}
	}

	return TEAM_NONE;
}

static SDL_Color chat_team_to_color(Team team) {
	switch (team) {
		case TEAM_RED:
			return CHAT_TEAM_RED_COLOR;
		case TEAM_BLUE:
			return CHAT_TEAM_BLUE_COLOR;
		default:
			return COL_WHITE;
	}
}

static int chat_extract_sender_prefix(const char* line, char* sender, int sender_size, int* prefix_len) {
	if (!line || !sender || sender_size <= 1 || !prefix_len) return EXIT_FAILURE;

	const char* separator = strstr(line, " : ");
	if (!separator || separator <= line) return EXIT_FAILURE;

	int sender_len = (int)(separator - line);
	if (sender_len <= 0 || sender_len >= sender_size) return EXIT_FAILURE;

	memcpy(sender, line, (size_t)sender_len);
	sender[sender_len] = '\0';

	// Inclut " :" pour colorer le pseudo et son séparateur immédiat.
	*prefix_len = sender_len + 2;
	return EXIT_SUCCESS;
}

static Text* chat_get_colored_prefix_text(AppContext* context, const char* font_path, int font_size, Uint8 opacity) {
	if (!context || !font_path || font_path[0] == '\0' || font_size <= 0) return NULL;

	if (!chat_colored_prefix_text) {
		TextConfig cfg = create_text_config(font_path, font_size, COL_WHITE, 0, 0, 0.0, opacity);
		chat_colored_prefix_text = init_text(context, " ", cfg);
		return chat_colored_prefix_text;
	}

	if (chat_colored_prefix_text->cfg.font_size != font_size ||
		!chat_font_path_equals(chat_colored_prefix_text->cfg.font_path, font_path)) {
		chat_colored_prefix_text->cfg.font_path = font_path;
		chat_colored_prefix_text->cfg.font_size = font_size;
		reload_text(context, chat_colored_prefix_text);
	}

	chat_colored_prefix_text->cfg.opacity = opacity;
	return chat_colored_prefix_text;
}

static int chat_render_sender_prefix(AppContext* context, const Window* chat_window, const char* prefix_text, Team sender_team, int rel_y, Text* colored_prefix) {
	if (!context || !chat_window || !prefix_text || !colored_prefix) return 0;
	if (sender_team != TEAM_RED && sender_team != TEAM_BLUE) return 0;

	update_text_color(context, colored_prefix, chat_team_to_color(sender_team));
	if (!colored_prefix->content || strcmp(colored_prefix->content, prefix_text) != 0) {
		update_text(context, colored_prefix, prefix_text);
	}

	int prefix_width = chat_text_width(colored_prefix);
	int prefix_rel_x = chat_left_anchored_rel_x(chat_window, prefix_width);
	window_place_text(chat_window, colored_prefix, prefix_rel_x, rel_y);
	display_text(context, colored_prefix);

	return prefix_width;
}

static void chat_wrap_cache_invalidate(Chat* chat) {
	if (!chat) return;
	memset(&chat->wrap_cache, 0, sizeof(ChatWrapCache));
}

static void chat_touch(Chat* chat) {
	if (!chat) return;
	chat->revision++;
	chat->wrap_cache.is_valid = 0;
}

static void chat_set_font_path(char* dst, int dst_size, const char* src) {
	if (!dst || dst_size <= 0) return;

	const char* value = src ? src : "";
	strncpy(dst, value, (size_t)dst_size - 1);
	dst[dst_size - 1] = '\0';
}

int chat_init(Chat* chat, int max_messages) {
	if (!chat || max_messages <= 0) return EXIT_FAILURE;

	chat->messages = list_create();
	if (!chat->messages) return EXIT_FAILURE;

	chat->max_messages = max_messages;
	chat->revision = 1;
	chat_wrap_cache_invalidate(chat);
	return EXIT_SUCCESS;
}

int chat_push(Chat* chat, const char* message) {
	if (!chat || !chat->messages || !message) return EXIT_FAILURE;

	char* copy = strdup(message);
	if (!copy) return EXIT_FAILURE;

	if (list_add(chat->messages, copy) != EXIT_SUCCESS) {
		free(copy);
		return EXIT_FAILURE;
	}

	while (chat->messages->size > chat->max_messages) {
		ListNode* oldest = chat->messages->head;
		if (!oldest) break;

		chat->messages->head = oldest->next;
		chat->messages->size--;

		free_chat_message(oldest->data);
		free(oldest);
	}

	chat_touch(chat);

	return EXIT_SUCCESS;
}

const char* chat_get(Chat* chat, int index) {
	if (!chat || !chat->messages) return NULL;
	return (const char*)list_get(chat->messages, index);
}

int chat_size(Chat* chat) {
	if (!chat || !chat->messages) return 0;
	return list_size(chat->messages);
}

void chat_clear(Chat* chat) {
	if (!chat) return;

	if (chat->messages) {
		list_destroy(chat->messages, free_chat_message);
		chat->messages = NULL;
	}

	if (chat_colored_prefix_text) {
		destroy_text(chat_colored_prefix_text);
		chat_colored_prefix_text = NULL;
	}

	chat->max_messages = 0;
	chat_touch(chat);
}

void chat_submit_message(AppContext* context, const char* text) {
	if (!context) return;

	printf("Chat input submitted: %s\n", text ? text : "");
	if (text && text[0] != '\0') {
		char msg[512];
		format_to(msg, sizeof(msg), "%d %s %s", MSG_SENDCHAT, context->player_name ? context->player_name : "Unknown", text);
		send_tcp(context->sock, msg);
	}
}

static int chat_append_wrapped_lines(const char* message, TTF_Font* font, int max_text_width, char out_lines[][CHAT_LINE_SIZE], int max_lines, int current_count) {
	if (!out_lines || max_lines <= 0 || current_count < 0 || current_count >= max_lines) return current_count;

	const char* cursor = (message && message[0] != '\0') ? message : " ";
	const char* message_start = cursor;
	const char* sender_separator = strstr(message_start, " : ");
	int min_first_line_len = 0;
	int is_first_chunk = 1;

	if (sender_separator && sender_separator > message_start && sender_separator[3] != '\0') {
		// Évite de couper juste après "pseudo :" quand on peut encore afficher au moins 1 caractère du contenu.
		min_first_line_len = (int)(sender_separator - message_start) + 4;
	}

	while (cursor[0] != '\0' && current_count < max_lines) {
		if (!font) {
			strncpy(out_lines[current_count], cursor, CHAT_LINE_SIZE - 1);
			out_lines[current_count][CHAT_LINE_SIZE - 1] = '\0';
			current_count++;
			break;
		}

		int full_width = 0;
		if (TTF_SizeUTF8(font, cursor, &full_width, NULL) != 0 || full_width <= max_text_width) {
			strncpy(out_lines[current_count], cursor, CHAT_LINE_SIZE - 1);
			out_lines[current_count][CHAT_LINE_SIZE - 1] = '\0';
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
			if (candidate_len >= CHAT_LINE_SIZE) {
				candidate_len = CHAT_LINE_SIZE - 1;
			}

			char probe[CHAT_LINE_SIZE];
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

			if (candidate_len >= CHAT_LINE_SIZE - 1) {
				break;
			}

			i_char += char_len;
		}

		int split_at = (best_space_break > 0) ? best_space_break : best_fit_len;
		if (is_first_chunk && min_first_line_len > 0 && split_at < min_first_line_len && best_fit_len >= min_first_line_len) {
			split_at = best_fit_len;
		}
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
		if (write_len >= CHAT_LINE_SIZE) {
			write_len = CHAT_LINE_SIZE - 1;
		}

		memcpy(out_lines[current_count], cursor, write_len);
		out_lines[current_count][write_len] = '\0';
		current_count++;

		cursor += split_at;
		while (cursor[0] == ' ') {
			cursor++;
		}
		is_first_chunk = 0;
	}

	return current_count;
}

static int chat_build_wrapped_lines_cached(Chat* chat, const char* font_path, int font_size, int max_text_width) {
	if (!chat) return 0;

	if (max_text_width < 1) {
		max_text_width = 1;
	}

	ChatWrapCache* cache = &chat->wrap_cache;
	unsigned int source_revision = chat->revision;

	int cache_hit =
		cache->is_valid &&
		cache->source_revision == source_revision &&
		cache->max_text_width == max_text_width &&
		cache->font_size == font_size &&
		chat_font_path_equals(cache->font_path, font_path);

	if (cache_hit) {
		return cache->total_lines;
	}

	TTF_Font* chat_font = NULL;
	if (font_path && font_path[0] != '\0' && font_size > 0) {
		chat_font = TTF_OpenFont(font_path, font_size);
	}

	const int total_messages = chat_size(chat);
	int total_lines = 0;
	for (int i_msg = 0; i_msg < total_messages && total_lines < CHAT_MAX_RENDER_LINES; i_msg++) {
		const char* message = chat_get(chat, i_msg);
		total_lines = chat_append_wrapped_lines(message, chat_font, max_text_width, cache->lines, CHAT_MAX_RENDER_LINES, total_lines);
	}

	if (chat_font) {
		TTF_CloseFont(chat_font);
	}

	if (total_lines <= 0) {
		cache->lines[0][0] = '\0';
		total_lines = 1;
	}

	cache->source_revision = source_revision;
	cache->max_text_width = max_text_width;
	cache->font_size = font_size;
	chat_set_font_path(cache->font_path, CHAT_FONT_PATH_MAX, font_path);
	cache->total_lines = total_lines;
	cache->is_valid = 1;

	return total_lines;
}

void chat_render_messages(AppContext* context, Window* chat_window, Text** chat_texts, int visible_lines) {
	if (!context || !context->lobby || !chat_window || !chat_window->cfg || !chat_texts || visible_lines <= 0) return;

	Chat* chat = &context->lobby->chat;
	if (!chat || !chat->messages) return;

	// La ligne la plus recente reste en bas du chat.
	int max_text_width = chat_window->cfg->w - CHAT_LEFT_PADDING - CHAT_RIGHT_PADDING;
	if (max_text_width < 32) max_text_width = 32;

	const char* font_path = NULL;
	int font_size = 0;
	Uint8 font_opacity = 255;
	if (chat_texts[0]) {
		font_path = chat_texts[0]->cfg.font_path;
		font_size = chat_texts[0]->cfg.font_size;
		font_opacity = chat_texts[0]->cfg.opacity;
	}

	Text* colored_prefix = chat_get_colored_prefix_text(context, font_path, font_size, font_opacity);

	int total_lines = chat_build_wrapped_lines_cached(chat, font_path, font_size, max_text_width);
	const char (*lines)[CHAT_LINE_SIZE] = chat->wrap_cache.lines;

	int max_scroll_offset = total_lines - visible_lines;
	if (max_scroll_offset < 0) max_scroll_offset = 0;

	if (chat_window->cfg->scroll_min != 0) {
		window_edit_cfg(chat_window, WIN_CFG_SCROLL_MIN, 0);
	}
	if (chat_window->cfg->scroll_max != max_scroll_offset) {
		window_edit_cfg(chat_window, WIN_CFG_SCROLL_MAX, max_scroll_offset);
	}

	int scroll_offset = chat_window->cfg->scroll_offset;
	if (scroll_offset < 0) scroll_offset = 0;
	if (scroll_offset > max_scroll_offset) scroll_offset = max_scroll_offset;

	const int rendered_lines = (total_lines < visible_lines) ? total_lines : visible_lines;
	int start_index = total_lines - rendered_lines - scroll_offset;
	if (start_index < 0) start_index = 0;

	SDL_Rect chat_clip = {0};
	int has_clip = (window_get_scrollable_zone_rect(chat_window, &chat_clip) == EXIT_SUCCESS);
	if (has_clip) {
		SDL_RenderSetClipRect(context->renderer, &chat_clip);
	}

	for (int i = 0; i < visible_lines; i++) {
		Text* txt = chat_texts[i];
		if (!txt) continue;

		if (i >= rendered_lines || (start_index + i) >= total_lines) {
			if (!txt->content || strcmp(txt->content, " ") != 0) {
				update_text(context, txt, " ");
			}
			continue;
		}

		const char* line = lines[start_index + i];
		const char* safe_line = line ? line : " ";
		const char* white_line = safe_line;
		int white_left_offset = 0;
		int rel_y = CHAT_BOTTOM_LINE_Y + ((rendered_lines - 1 - i) * CHAT_LINE_GAP);

		if (colored_prefix) {
			char sender_name[CHAT_LINE_SIZE];
			int prefix_len = 0;
			if (chat_extract_sender_prefix(safe_line, sender_name, CHAT_LINE_SIZE, &prefix_len) == EXIT_SUCCESS) {
				Team sender_team = chat_find_team_by_player_name(context, sender_name);
				if (sender_team == TEAM_RED || sender_team == TEAM_BLUE) {
					if (prefix_len >= CHAT_LINE_SIZE) {
						prefix_len = CHAT_LINE_SIZE - 1;
					}

					char prefix_text[CHAT_LINE_SIZE];
					memcpy(prefix_text, safe_line, (size_t)prefix_len);
					prefix_text[prefix_len] = '\0';

					white_line = safe_line + prefix_len;
					if (!white_line || white_line[0] == '\0') {
						white_line = " ";
					}

					white_left_offset = chat_render_sender_prefix(
						context,
						chat_window,
						prefix_text,
						sender_team,
						rel_y,
						colored_prefix
					);
				}
			}
		}

		if (!txt->content || strcmp(txt->content, white_line) != 0) {
			update_text(context, txt, white_line);
		}

		// window_place_text centre le texte: on compense avec text_w/2 pour ancrer le bord gauche.
		int rel_x = chat_left_anchored_rel_x_with_offset(chat_window, chat_text_width(txt), white_left_offset);
		window_place_text(chat_window, txt, rel_x, rel_y);
		display_text(context, txt);
	}

	if (has_clip) {
		SDL_RenderSetClipRect(context->renderer, NULL);
	}
}
