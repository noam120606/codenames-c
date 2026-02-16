#include "../lib/all.h"

int main(int argc, char* argv[]){

    const float target_fps = 60.0f;
    char ip[16] = "127.0.0.1";
    int port = 0;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
            case 's':
                strncpy(ip, optarg, sizeof(ip) - 1);
                ip[sizeof(ip) - 1] = '\0';
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-s server_ip] [-p port]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (port == 0) {
        fprintf(stderr, "Port number is required. Usage: %s [-s server_ip] [-p port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Starting Codenames Client...\n");
    
    // Initialize TCP connection to server
    int sock = init_tcp(ip, port);
    if (sock < 0) {
        printf("Failed to connect to the server at %s:%d\n", ip, port);
        return EXIT_FAILURE;
    }

    SDL_Context context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        close_tcp(sock);
        destroy_context(context);
        return EXIT_FAILURE;
    }

    printf("Connected to server at %s:%d\n", ip, port);

    // Initialiser le système de boutons
    buttons_init();

    int menu_loading_fails = menu_init(&context);
    if (menu_loading_fails > 0) {
        printf("Failed to load %d menu resource(s)\n", menu_loading_fails);
        close_tcp(sock);
        menu_free();
        destroy_context(context);
        return EXIT_FAILURE;
    }
    int background_loading_fails = init_background(&context);
    if (background_loading_fails > 0) {
        printf("Failed to load %d background resource(s)\n", background_loading_fails);
        close_tcp(sock);
        menu_free();
        destroy_context(context);
        destroy_background();
        return EXIT_FAILURE;
    }
    int infos_loading_fails = init_infos(&context);
    if (infos_loading_fails > 0) {
        printf("Failed to load %d infos resource(s)\n", infos_loading_fails);
        close_tcp(sock);
        menu_free();
        destroy_context(context);
        destroy_background();
        infos_free();
        return EXIT_FAILURE;
    }

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(sock) == EXIT_SUCCESS) {
        // Enregistrer le début de la frame
        context.frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            // Gestion bouttons
            ButtonReturn result = buttons_handle_event(context, &e);
            switch (result) {
                case BTN_RET_NONE: break;
                case BTN_RET_QUIT: running = 0; break;
                default: break;
            }
            // Menu events (inputs, etc.)
            menu_handle_event(&context, &e);
        }

        // Pré Rendu
        SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
        SDL_RenderClear(context.renderer);

        // Rendu et logique d'affichage
        display_background(&context);
        menu_display(&context);
        infos_display(&context);

        // Afficher les boutons
        buttons_display(context.renderer);

        // Post Rendu
        SDL_RenderPresent(context.renderer);
        
        // Calculer le temps écoulé pour cette frame
        Uint32 frame_end_time = SDL_GetTicks();
        Uint32 frame_elapsed = frame_end_time - context.frame_start_time;
        
        // Calculer le délai nécessaire pour atteindre la durée cible
        float frame_duration = 1000.0f / target_fps; // Cible de 60 FPS = 16.67ms par frame
        int frame_delay = (int)(frame_duration - frame_elapsed);
        if (frame_delay > 0) SDL_Delay(frame_delay);
        
        context.clock++;
        
    }

    printf("Exiting...\n");

    close_tcp(sock);
    menu_free();
    destroy_background();
    infos_free();
    buttons_free();
    destroy_context(context);

    return EXIT_SUCCESS;
}