#include "../lib/all.h"

MessageType fetch_header(char* message) {
    MessageType header;
    if (!sscanf(message, "%d", (int*)&header)) return MSG_UNKNOWN;
    return header;
}

Arguments parse_arguments(char* message) {
    Arguments args;
    char* token = strtok(message, " ");
    args.argc = 0;
    args.argv = NULL;

    while (token != NULL) {
        args.argc++;
        args.argv = realloc(args.argv, args.argc * sizeof(char*));
        ((char**)args.argv)[args.argc - 1] = token;
        token = strtok(NULL, " ");
    }
    return args;
}

static SDL_Color message_hint_bar_team_color(Team team) {
    switch (team) {
        case TEAM_BLUE:
            return (SDL_Color){50, 80, 150, 200};
        case TEAM_RED:
            return (SDL_Color){150, 50, 50, 200};
        default:
            return COL_GRAY;
    }
}

static int message_find_user_slot_by_id(const Lobby* lobby, int user_id) {
    if (!lobby || user_id < 0) return -1;

    for (int i = 0; i < MAX_USERS; i++) {
        User* user = lobby->users[i];
        if (user && user->id == user_id) {
            return i;
        }
    }

    return -1;
}

static int message_find_free_user_slot(const Lobby* lobby) {
    if (!lobby) return -1;

    for (int i = 0; i < MAX_USERS; i++) {
        if (!lobby->users[i]) {
            return i;
        }
    }

    return -1;
}

static int message_update_user_name(User* user, const char* name) {
    if (!user || !name || name[0] == '\0') return EXIT_SUCCESS;

    if (user->name && strcmp(user->name, name) == 0) {
        return EXIT_SUCCESS;
    }

    char* copy = strdup(name);
    if (!copy) return EXIT_FAILURE;

    free(user->name);
    user->name = copy;
    return EXIT_SUCCESS;
}

static User* message_upsert_lobby_user(Lobby* lobby, int user_id, const char* name, UserRole role, Team team) {
    if (!lobby || user_id < 0) return NULL;

    int slot = message_find_user_slot_by_id(lobby, user_id);
    if (slot >= 0) {
        User* user = lobby->users[slot];
        if (!user) return NULL;

        if (message_update_user_name(user, name) != EXIT_SUCCESS) {
            return NULL;
        }

        user->role = role;
        user->team = team;
        return user;
    }

    int free_slot = message_find_free_user_slot(lobby);
    if (free_slot < 0) {
        return NULL;
    }

    const char* safe_name = (name && name[0] != '\0') ? name : "Unknown";
    User* created = create_user(user_id, safe_name, role, team);
    if (!created) {
        return NULL;
    }

    lobby->users[free_slot] = created;
    lobby->nb_players++;
    return created;
}

static void message_sync_local_user_in_lobby(AppContext* context) {
    if (!context || !context->lobby || context->player_id < 0) return;

    const char* local_name = (context->player_name && context->player_name[0] != '\0') ? context->player_name : "Unknown";
    if (!message_upsert_lobby_user(context->lobby, context->player_id, local_name, context->player_role, context->player_team)) {
        printf("Failed to synchronize local player %d in lobby user list\n", context->player_id);
    }
}

int on_message(AppContext* context, char* message) {
    MessageType header = fetch_header(message);
    message += number_length((int)header) + 1; // Skip header et espace

     /* Keep a raw copy of the message (everything after the header)
         so we can print the full argument string later even though
         parse_arguments() will modify `message` with strtok(). */
     char* raw_message = strdup(message);
     Arguments args = parse_arguments(message);

    switch (header) {

        case MSG_SERVER_ERROR:
            if (args.argc < 1) {
                printf("Invalid error message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }
            printf("Error from server: %s\n", (char*)args.argv[0]);
            break;
            
        case MSG_UNKNOWN: 
            printf("Received unknown message from server: \"%s\"\n", message);
            break;

        case MSG_INFO:
            if (args.argc < 1) {
            printf("Invalid info message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            /* Print the entire argument string (everything after header) */
            printf("Info from server: %s\n", raw_message ? raw_message : "");

            break;

        case MSG_CREATELOBBY: // Confirmation de la création du lobby, avec l'id du lobby créé
            if (args.argc < 2) {
                printf("Invalid create lobby message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }
            struct_lobby_init(context->lobby, atoi((char*)args.argv[0]), (char*)args.argv[1]);
            // Le créateur du lobby en est automatiquement le propriétaire
            context->lobby->owner_id = context->player_id;
            message_sync_local_user_in_lobby(context);
            break;

        case MSG_JOINLOBBY:
            // Handle join lobby
            if (args.argc >= 2) {
                struct_lobby_init(context->lobby, atoi((char*)args.argv[0]), (char*)args.argv[1]);
                message_sync_local_user_in_lobby(context);
            }
            break;

        case MSG_LEAVELOBBY:
            // Handle leave lobby
            context->lobby->id = -1;
            break;

        case MSG_LOBBYCLOSED:
            // Handle lobby closed
            context->lobby->id = -1;
            break;
        
        case MSG_PLAYERJOINED: {
            if (args.argc < 4) {
                printf("Invalid player joined message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int joined_id = atoi((char*)args.argv[0]);
            const char* joined_name = (char*)args.argv[1];
            UserRole joined_role = (UserRole)atoi((char*)args.argv[2]);
            Team joined_team = (Team)atoi((char*)args.argv[3]);

            if (!message_upsert_lobby_user(context->lobby, joined_id, joined_name, joined_role, joined_team)) {
                printf("Failed to add/update player %d in lobby\n", joined_id);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            printf("Player %d (%s) joined the lobby with role %d and team %d\n", joined_id, joined_name ? joined_name : "Unknown", joined_role, joined_team);
            break;
        }
        
        case MSG_PLAYERLEFT: {
            if (args.argc < 1) {
                printf("Invalid player left message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }
            int player_id = atoi((char*)args.argv[0]);
            for (int i = 0; i < MAX_USERS; i++) {
                if (context->lobby->users[i] && context->lobby->users[i]->id == player_id) {
                    destroy_user(context->lobby->users[i]);
                    context->lobby->users[i] = NULL;
                    context->lobby->nb_players--;
                    break;
                }
            }
            printf("Player %d left the lobby\n", player_id);
            break;
        }

        case MSG_CHOOSE_ROLE: {
            if (args.argc < 3) {
                printf("Invalid choose role message from server : \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int target_id = atoi((char*)args.argv[0]);
            UserRole target_role = (UserRole)atoi((char*)args.argv[1]);
            Team target_team = (Team)atoi((char*)args.argv[2]);
            const char* target_name = find_player_by_id(context->lobby, target_id);
            if (!target_name && context->player_id == target_id) {
                target_name = context->player_name;
            }

            if (!message_upsert_lobby_user(context->lobby, target_id, target_name, target_role, target_team)) {
                printf("Failed to update role/team for player %d\n", target_id);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            if (context->player_id == target_id) {
                context->player_role = target_role;
                context->player_team = target_team;
            }

            printf("Player %d chose role %d team %d\n", target_id, target_role, target_team);
            
            break;
        }

        case MSG_STARTGAME: {
            if (args.argc < 2) {
                printf("Invalid start game message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            if (context->lobby->game) game_struct_free(context);

            Game* game = (Game*)calloc(1, sizeof(Game));
            if (!game) {
                printf("Failed to allocate memory for game\n");
                return EXIT_FAILURE;
            }
            game->state = (GameState)atoi((char*)args.argv[0]);
            game->nb_words = atoi((char*)args.argv[1]);
            game->current_hint[0] = '\0';
            game->current_hint_count = 0;
            game->winner = TEAM_NONE;
            history_reset(&game->red_history);
            history_reset(&game->blue_history);
            printf("Starting game with state %d and %d words\n", game->state, game->nb_words);
            game->cards = (Card*)malloc(sizeof(Card) * game->nb_words);
            if (!game->cards) {
                printf("Failed to allocate memory for game cards\n");
                free(game);
                return EXIT_FAILURE;
            }

            context->lobby->game = game;

            printf("Game started with status %d\n", game->state);
            break;
        }

        case MSG_WORDDATA: {
            if (args.argc < 4) {
                printf("Invalid word data message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int wordid = atoi((char*)args.argv[0]);
            char* word = (char*)args.argv[1];
            Team team = (Team)atoi((char*)args.argv[2]);
            int revealed = atoi((char*)args.argv[3]);

            printf("Word data received: %s (Team: %d, Revealed: %d)\n", word, team, revealed);

            // Prétraitement qui remet les espaces
            for (int i = 0; word[i] != '\0'; i++) {
                if (word[i] == '_') word[i] = ' ';
            }
            
            strcpy(context->lobby->game->cards[wordid].word, word);
            context->lobby->game->cards[wordid].team = team;
            context->lobby->game->cards[wordid].type = rand() % 4; // 0 = homme, 1 = femme
            context->lobby->game->cards[wordid].revealed = revealed;
            context->lobby->game->cards[wordid].selected = False;
            context->lobby->game->cards[wordid].is_pressed = False;
            context->lobby->game->cards[wordid].is_hovered = False;

            if (wordid == context->lobby->game->nb_words - 1) {
                context->app_state = APP_STATE_PLAYING;
            }

            break;
        }

        case MSG_SUBMIT_HINT: {
            if (args.argc < 4) {
                printf("Invalid submit hint message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int spy_id = atoi((char*)args.argv[0]);
            int nb_guesses = atoi((char*)args.argv[1]);
            char* hint = (char*)args.argv[2];
            GameState new_state = (GameState)atoi((char*)args.argv[3]);
            GameState previous_state = GAMESTATE_WAITING;

            const char* spy_name = find_player_by_id(context->lobby, spy_id); //Récupérer le nom de l'espion

            if (context->lobby && context->lobby->game) {
                previous_state = context->lobby->game->state;
            }

            Team active_team = history_team_from_agent_state(new_state);

            printf("Hint received from client %d (%s) : %s with %d guesses, new state: %d\n", spy_id, spy_name ? spy_name : "l'espion", hint, nb_guesses, new_state);

            // Stocker l'indice et mettre à jour le gamestate
            if (context->lobby && context->lobby->game) {
                strncpy(context->lobby->game->current_hint, hint, sizeof(context->lobby->game->current_hint) - 1);
                context->lobby->game->current_hint[sizeof(context->lobby->game->current_hint) - 1] = '\0';
                context->lobby->game->current_hint_count = nb_guesses;
                context->lobby->game->state = new_state;

                if (active_team != TEAM_NONE) {
                    int should_start_turn = 0;
                    if (active_team == TEAM_RED && previous_state == GAMESTATE_TURN_RED_SPY) {
                        should_start_turn = 1;
                    } else if (active_team == TEAM_BLUE && previous_state == GAMESTATE_TURN_BLUE_SPY) {
                        should_start_turn = 1;
                    } else {
                        History* history = history_get_for_team(context->lobby->game, active_team);
                        if (!history || history->turn_count <= 0) {
                            should_start_turn = 1;
                        }
                    }

                    if (should_start_turn) {
                        history_start_turn(context, active_team, hint, nb_guesses);
                    } else {
                        history_ensure_turn(context, active_team, hint, nb_guesses);
                    }

                    /* Le serveur fournit le nom de l'espion: on l'applique systématiquement au dernier tour. */
                    History* history = history_get_for_team(context->lobby->game, active_team);
                    if (history && history->turn_count > 0) {
                        history_update_last_turn(history, spy_name, hint, nb_guesses);
                    }
                }

                int is_submitting_spy = 0;
                if (context->player_id >= 0 && context->player_id == spy_id) {
                    is_submitting_spy = 1;
                } else if (context->player_name && spy_name && strcmp(context->player_name, spy_name) == 0) {
                    is_submitting_spy = 1;
                }

                if (!is_submitting_spy) {
                    char hint_feedback[GAME_HINTBAR_TEXT_LEN];
                    format_to(
                        hint_feedback,
                        sizeof(hint_feedback),
                        "Indice reçu de %s",
                        spy_name ? spy_name : "l'espion"
                    );
                    game_hint_bar_set_feedback(
                        context,
                        hint_feedback,
                        message_hint_bar_team_color(active_team),
                        GAME_HINTBAR_PRIORITY_INFO,
                        GAME_HINTBAR_FEEDBACK_INFO_MS
                    );
                }
            }
            
            break;
        }

        case MSG_PREGUESS: {
            if (args.argc < 3) {
                printf("Invalid preguess message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            printf("Pre-guess update received: \"%s\"\n", raw_message ? raw_message : "");

            int word_index = atoi((char*)args.argv[0]);
            int selected = atoi((char*)args.argv[1]);
            int playerid = atoi((char*)args.argv[2]);
            (void)playerid; // Utile plus tard

            printf("Pre-guess update received for card %d: selected = %d\n", word_index, selected);

            // Mettre à jour la carte
            if (context->lobby && context->lobby->game && word_index >= 0 && word_index < context->lobby->game->nb_words) {
                Card* card = &context->lobby->game->cards[word_index];
                card->selected = selected;
            }
            
            break;
        }

        case MSG_GUESS_CARD: {
            if (args.argc < 2) {
                printf("Invalid guess card message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int word_index = atoi((char*)args.argv[0]);
            GameState new_state = (GameState)atoi((char*)args.argv[1]);
            const char* guessing_agent_name = NULL;
            if (args.argc >= 4) {
                guessing_agent_name = (char*)args.argv[3];
            } else if (word_index == -1 && args.argc >= 3) {
                guessing_agent_name = (char*)args.argv[2];
            }
            Team active_team = TEAM_NONE;
            if (context->lobby && context->lobby->game) {
                active_team = history_team_from_agent_state(context->lobby->game->state);
            }

            // Partie terminée
            if (args.argc >= 3 && new_state == GAMESTATE_ENDED) {
                context->lobby->game->winner = (Team)atoi((char*)args.argv[2]);

                if (
                    (context->lobby->game->winner == TEAM_RED && context->player_team == TEAM_RED) ||
                    (context->lobby->game->winner == TEAM_BLUE && context->player_team == TEAM_BLUE)
                ) {
                    char nb_win[16];
                    read_property(nb_win, "WIN_COUNT");
                    int win_count = strcmp(nb_win, "")!=0 ? atoi(nb_win) + 1 : 1;
                    format_to(nb_win, sizeof(nb_win), "%d", win_count);
                    write_property("WIN_COUNT", nb_win);
                }

                char end_feedback[GAME_HINTBAR_TEXT_LEN];
                const char* winner_label = "inconnue";
                if (context->lobby->game->winner == TEAM_BLUE) {
                    winner_label = "bleue";
                } else if (context->lobby->game->winner == TEAM_RED) {
                    winner_label = "rouge";
                }
                format_to(
                    end_feedback,
                    sizeof(end_feedback),
                    "Victoire de l'equipe %s !",
                    winner_label
                );
                game_hint_bar_set_feedback(
                    context,
                    end_feedback,
                    message_hint_bar_team_color(context->lobby->game->winner),
                    GAME_HINTBAR_PRIORITY_INFO,
                    GAME_HINTBAR_FEEDBACK_SUCCESS_MS
                );
                
            } 

            if (new_state != context->lobby->game->state) {
                for (int i = 0; i < context->lobby->game->nb_words; i++) {
                    context->lobby->game->cards[i].selected = False;
                }
            }

            printf(
                "Card guessed: %d by %s, new state: %d\n",
                word_index,
                guessing_agent_name ? guessing_agent_name : "unknown",
                new_state
            );

            // Mettre à jour la carte et le gamestate
            if (context->lobby && context->lobby->game) {
                if (word_index >= 0 && word_index < context->lobby->game->nb_words) {
                    Card* guessed_card = &context->lobby->game->cards[word_index];

                    if (active_team != TEAM_NONE) {
                        history_append_revealed_word(
                            context,
                            active_team,
                            guessed_card->word,
                            guessed_card->team,
                            guessing_agent_name
                        );
                    }

                    guessed_card->revealed = True;
                    guessed_card->is_hovered = False;
                    guessed_card->selected = False;
                } else if (word_index == -1 && active_team != TEAM_NONE) {
                    history_ensure_turn(
                        context,
                        active_team,
                        context->lobby->game->current_hint,
                        context->lobby->game->current_hint_count
                    );

                    char turn_feedback[GAME_HINTBAR_TEXT_LEN];
                    format_to(
                        turn_feedback,
                        sizeof(turn_feedback),
                        "Tour termine par %s",
                        guessing_agent_name ? guessing_agent_name : "un agent"
                    );
                    game_hint_bar_set_feedback(
                        context,
                        turn_feedback,
                        message_hint_bar_team_color(active_team),
                        GAME_HINTBAR_PRIORITY_INFO,
                        GAME_HINTBAR_FEEDBACK_INFO_MS
                    );
                }
                context->lobby->game->state = new_state;
            }
            
            break;
        }

        case MSG_SENDCHAT: {
            if (args.argc < 2) {
                printf("Invalid chat message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            char* sender = (char*)args.argv[0];
            char chat_message[448];
            chat_message[0] = '\0';
            for (int i = 1; i < args.argc; i++) {
                if (i > 1) {
                    strncat(chat_message, " ", sizeof(chat_message) - strlen(chat_message) - 1);
                }
                strncat(chat_message, (char*)args.argv[i], sizeof(chat_message) - strlen(chat_message) - 1);
            }

            char full_message[512];
            format_to(full_message, sizeof(full_message), "%s : %s", sender, chat_message);
            if (chat_push(&context->lobby->chat, full_message) != EXIT_SUCCESS) {
                printf("Failed to store chat message in lobby history\n");
            }

            printf("%s : %s\n", sender, chat_message);
            
            break;
        }

        case MSG_REQUESTUUID: {
            // Réception de l'UUID généré par le serveur
            if (args.argc < 1) {
                printf("Invalid UUID message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }
            // Stocker l'UUID dans le contexte
            if (context->player_uuid) free(context->player_uuid);
            context->player_uuid = strdup((char*)args.argv[0]);

            // Écrire le fichier data/uuid
            
            FILE* f = fopen("data/uuid", "w");
            if (f) {
                fprintf(f, "NE PAS MODIFIER CE FICHIER !\n%s\n", context->player_uuid);
                fclose(f);
                printf("UUID saved to data/uuid: %s\n", context->player_uuid);
            } else {
                perror("Failed to write data/uuid");
            }
            
            break;
        }

        case MSG_SEND_CLIENT_ID: {
            if (args.argc < 1) {
                printf("Invalid client id message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            context->player_id = atoi((char*)args.argv[0]);
            message_sync_local_user_in_lobby(context);
            printf("Assigned client ID: %d\n", context->player_id);

            break;
        }

        case MSG_PING: {
            if (args.argc < 1) {
                break;
            }
            
            Uint32 sent_at = (Uint32)strtoul((char*)args.argv[0], NULL, 10);
            Uint32 now = SDL_GetTicks();
            context->ping_ms = (int)(now - sent_at);
            
            break;
        }

        case MSG_SET_WORDS_DIFFICULTY: {
            if (args.argc < 1) {
                printf("Invalid set words difficulty message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int words_difficulty = atoi((char*)args.argv[0]);
            context->lobby->words_difficulty = (WordsDifficulty)words_difficulty;
            printf("Lobby words difficulty changed to %s\n", words_difficulty == WORDS_DIFFICULTY_HARD ? "hard" : "easy");

            break;
        }

        case MSG_SET_NB_ASSASSINS: {
            if (args.argc < 1) {
                printf("Invalid set nb_assassins message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            int nb_assassins = atoi((char*)args.argv[0]);
            if (nb_assassins < 1 || nb_assassins > 3) {
                printf("Invalid nb_assassins value from server: %d\n", nb_assassins);
                break;
            }

            if (context->lobby) {
                context->lobby->nb_assassins = nb_assassins;
            }
            printf("Lobby nb_assassins changed to %d\n", nb_assassins);

            break;
        }

        default:
            printf("Received unhandled message type %d from server: \"%s\"\n", header, message);
            break;
    };

    if (args.argv) free(args.argv);
    if (raw_message) free(raw_message);

    return EXIT_SUCCESS;
}  