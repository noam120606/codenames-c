#include "../lib/all.h"

int main(int argc, char* argv[]){

    const float target_fps = 60.0f;
    char ip[16] = "127.0.0.1";
    int port = 0;
    /* Nombre de frames avant fermeture automatique (0 pour désactiver)
    * - peut être configuré via la variable d'environnement CODENAMES_AUTOCLOSE_FRAMES
    * utile pour les tests de fuite mémoire afin de ne pas laisser la fenêtre ouverte indéfiniment.
    */
    int auto_close_frames = 0; 
    const char* auto_close_env = getenv("CODENAMES_AUTOCLOSE_FRAMES");

    if (auto_close_env) {
        auto_close_frames = atoi(auto_close_env);
        if (auto_close_frames < 0) auto_close_frames = 0;
    }

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

    Resources* resources = init_resources();
    if (!resources) {
        printf("Failed to initialize resources\n");
        return EXIT_FAILURE;
    }

    SDL_Context context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    // Register SDL context and cleanup function
    if (define_sdl_context(resources, &context, destroy_context) != EXIT_SUCCESS) {
        printf("Failed to define SDL context\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    // Initialize TCP connection to server
    context.sock = init_tcp(ip, port);
    if (context.sock < 0) {
        printf("Failed to connect to the server at %s:%d\n", ip, port);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    printf("Connected to server at %s:%d\n", ip, port);

    // Initialiser le système de boutons
    buttons_init();
    add_destroy_resource(resources, buttons_free);

    int menu_loading_fails = menu_init(&context);
    if (menu_loading_fails > 0) {
        printf("Failed to load %d menu resource(s)\n", menu_loading_fails);
        menu_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, menu_free);

    int background_loading_fails = init_background(&context);
    if (background_loading_fails > 0) {
        printf("Failed to load %d background resource(s)\n", background_loading_fails);
        destroy_background();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, destroy_background);

    int infos_loading_fails = init_infos(&context);
    if (infos_loading_fails > 0) {
        printf("Failed to load %d infos resource(s)\n", infos_loading_fails);
        infos_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, infos_free);

    int lobby_loading_fails = lobby_init(&context);
    if (lobby_loading_fails > 0) {
        printf("Failed to load %d lobby resource(s)\n", lobby_loading_fails);
        lobby_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, lobby_free);
    int game_loading_fails = game_init(&context);
    if (game_loading_fails > 0) {
        printf("Failed to load %d game resource(s)\n", game_loading_fails);
        game_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, game_free);

    SDL_Event e;
    int running = 1;

    while (running && tick_tcp(&context, context.sock) == EXIT_SUCCESS) {
        // Enregistrer le début de la frame
        context.frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F11) {
                toggle_fullscreen(&context);
            }
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
                    toggle_fullscreen(&context);
                } else if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    Uint32 wflags = SDL_GetWindowFlags(context.window);
                    int is_fs = (wflags & SDL_WINDOW_FULLSCREEN_DESKTOP) || (wflags & SDL_WINDOW_FULLSCREEN);
                    if (is_fs) toggle_fullscreen(&context);
                }
            }
            /* Le bandeau infos (crossfaders volume) est toujours actif */
            infos_handle_event(&context, &e);

            /* Fond interactif (clic sur un symbole) */
            if (context.game_state == GAME_STATE_MENU || context.game_state == GAME_STATE_LOBBY)
                background_handle_event(&context, &e);

            // Déléguer l'événement selon l'état courant
            switch (context.game_state) {
                case GAME_STATE_MENU: menu_handle_event(&context, &e); break;
                case GAME_STATE_LOBBY: lobby_handle_event(&context, &e); break;
                case GAME_STATE_PLAYING: game_handle_event(&context, &e); break;
                case GAME_STATE_PAUSED: break;
            }

            ButtonReturn bref = buttons_handle_event(&context, &e);
            switch (bref) {
                case BTN_RET_QUIT: running = 0; break;
                default: break;
            }
        }

        // Pré Rendu
        SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
        SDL_RenderClear(context.renderer);

        // Rendu et logique d'affichage
        switch (context.game_state) {
            case GAME_STATE_MENU:
                display_background(&context);
                menu_display(&context);
                
                break;
            case GAME_STATE_LOBBY:
                display_background(&context);
                lobby_display(&context);
                break;
            case GAME_STATE_PLAYING:
                game_display(&context);
                break;
            case GAME_STATE_PAUSED:
                // TODO: Implement paused state rendering
                break;
        }

        // Onglet d'infos
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

        if (auto_close_frames > 0 && context.clock >= auto_close_frames) {
            running = 0;
        }
        
    }

    printf("Exiting...\n");

    cleanup_resources(resources);

    return EXIT_SUCCESS;
}