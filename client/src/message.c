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

int on_message(SDL_Context* context, char* message) {
    MessageType header = fetch_header(message);
    message += number_length((int)header) + 1; // Skip header et espace

    Arguments args = parse_arguments(message);

    printf("%s\n", message);

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
            context->lobby_id = atoi((char*)args.argv[0]);
            context->lobby_code = strdup((char*)args.argv[1]);
            break;

        case MSG_JOINLOBBY:
            // Handle join lobby
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