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

    Arguments args = parse_arguments(message);

    switch (header) {
        case MSG_UNKNOWN: 
            printf("Received unknown message from server: \"%s\"\n", message);
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
        
        case MSG_PLAYERJOINED:
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
        
        case MSG_PLAYERLEFT:
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

        case MSG_CHOOSE_ROLE:
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


        case MSG_REQUESTUUID:
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
            {
                FILE* f = fopen("data/uuid", "w");
                if (f) {
                    fprintf(f, "NE PAS MODIFIER CE FICHIER !\n%s\n", context->player_uuid);
                    fclose(f);
                    printf("UUID saved to data/uuid: %s\n", context->player_uuid);
                } else {
                    perror("Failed to write data/uuid");
                }
            }
            break;

        default:
            printf("Received unhandled message type %d from server: \"%s\"\n", header, message);
            break;
    };

    if (args.argv) free(args.argv);

    return EXIT_SUCCESS;
}  