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
            break;

        case MSG_JOINLOBBY:
            // Handle join lobby
            if (args.argc >= 2) {
                struct_lobby_init(context->lobby, atoi((char*)args.argv[0]), (char*)args.argv[1]);
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

            User* new_player = create_user(atoi((char*)args.argv[0]), (char*)args.argv[1], (UserRole)atoi((char*)args.argv[2]), (Team)atoi((char*)args.argv[3]));

            for (int i = 0; i < MAX_USERS; i++) {
                if (!context->lobby->users[i]) {
                    context->lobby->users[i] = new_player;
                    context->lobby->nb_players++;
                    break;
                }
            }

            printf("Player %s joined the lobby with role %s and team %s\n", (char*)args.argv[0], (char*)args.argv[1], (char*)args.argv[2]);
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

            for (int i = 0; i < MAX_USERS; i++) {
                if (context->lobby->users[i] && context->lobby->users[i]->id == atoi((char*)args.argv[0])) {
                    context->lobby->users[i]->role = (UserRole)atoi((char*)args.argv[1]);
                    context->lobby->users[i]->team = (Team)atoi((char*)args.argv[2]);
                    break;
                }
            }

            printf("Player %s chose role %s team %s\n", (char*)args.argv[0], (char*)args.argv[1], (char*)args.argv[2]);
            
            break;
        }

        case MSG_STARTGAME: {
            if (args.argc < 2) {
                printf("Invalid start game message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            Game* game = (Game*)malloc(sizeof(Game));
            if (!game) {
                printf("Failed to allocate memory for game\n");
                return EXIT_FAILURE;
            }
            game->state = (GameState)atoi((char*)args.argv[0]);
            game->nb_words = atoi((char*)args.argv[1]);
            printf("Starting game with state %d and %d words\n", game->state, game->nb_words);
            game->words = (Word*)malloc(sizeof(Word) * game->nb_words);
            if (!game->words) {
                printf("Failed to allocate memory for game words\n");
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

            strcpy(context->lobby->game->words[wordid].word, word);
            context->lobby->game->words[wordid].team = team;
            context->lobby->game->words[wordid].revealed = revealed;
            context->lobby->game->words[wordid].gender = rand() % 2; // 0 = homme, 1 = femme

            if (wordid == context->lobby->game->nb_words - 1) {
                context->app_state = APP_STATE_PLAYING;
            }

            break;
        }

        case MSG_SUBMIT_HINT: {
            if (args.argc < 2) {
                printf("Invalid submit hint message from server: \"%s\"\n", message);
                if (args.argv) free(args.argv);
                return EXIT_FAILURE;
            }

            char* hint = (char*)args.argv[0];
            int nb_guesses = atoi((char*)args.argv[1]);

            printf("Hint submitted: %s with %d guesses\n", hint, nb_guesses);
            
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
            format_to(full_message, sizeof(full_message), "%s: %s", sender, chat_message);
            if (chat_push(&context->lobby->chat, full_message) != EXIT_SUCCESS) {
                printf("Failed to store chat message in lobby history\n");
            }

            printf("%s: %s\n", sender, chat_message);
            
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

        case MSG_PING: {
            if (args.argc < 1) {
                break;
            }
            
            Uint32 sent_at = (Uint32)strtoul((char*)args.argv[0], NULL, 10);
            Uint32 now = SDL_GetTicks();
            context->ping_ms = (int)(now - sent_at);
            
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