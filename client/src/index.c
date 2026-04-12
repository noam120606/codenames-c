#include "../lib/all.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define GAME_WINDOW_ICON_PATH "assets/img/others/window_icon_big.png"

// Applique l'icône de la fenêtre de jeu.
static void set_game_window_icon(AppContext* context) {
    if (!context || !context->window) return;

    SDL_Surface* icon_surface = IMG_Load(GAME_WINDOW_ICON_PATH);
    if (!icon_surface) {
        fprintf(stderr, "Warning: failed to load window icon '%s': %s\n", GAME_WINDOW_ICON_PATH, IMG_GetError());
        return;
    }

    SDL_SetWindowIcon(context->window, icon_surface);
    SDL_FreeSurface(icon_surface);
}

// Relance le processus de jeu avec les mêmes arguments que le processus actuel.
static int relaunch_game_process(int argc, char* argv[]) {
#ifdef _WIN32
    (void)argc;
    (void)argv;

    char* command_line = _strdup(GetCommandLineA());
    if (!command_line) {
        return EXIT_FAILURE;
    }

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    ZeroMemory(&process_info, sizeof(process_info));
    startup_info.cb = sizeof(startup_info);

    // Lancer le nouveau processus en utilisant la ligne de commande complète du processus actuel pour préserver tous les arguments et options.
    BOOL launched = CreateProcessA(
        NULL,
        command_line,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );

    free(command_line);

    if (!launched) {
        fprintf(stderr, "Failed to relaunch game process (CreateProcessA error %lu)\n", (unsigned long)GetLastError());
        return EXIT_FAILURE;
    }

    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    return EXIT_SUCCESS;
#else
    if (!argv || argc <= 0 || !argv[0] || argv[0][0] == '\0') {
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed while relaunching game");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        execv(argv[0], argv);
        perror("execv failed while relaunching game");
        _exit(127);
    }

    return EXIT_SUCCESS;
#endif
}

// Applique les transitions audio une seule fois au changement d'état.
static void handle_app_state_audio_transition(AppContext* context, AppState previous_state, AppState new_state) {
    if (!context || previous_state == new_state) {
        return;
    }

    // Réapplique les volumes utilisateur pour éviter un volume de canal résiduel après un fade.
    audio_set_type_volume(AUDIO_SOUND_KIND_MUSIC, context->music_volume);
    audio_set_type_volume(AUDIO_SOUND_KIND_SFX, context->sound_effects_volume);

    if (new_state == APP_STATE_MENU || new_state == APP_STATE_LOBBY) {
        if (audio_is_playing(MUSIC_GAME)) {
            audio_stop_with_fade(MUSIC_GAME, 1000, AUDIO_FADE_OUT_BY_VOLUME, NULL);
        }
    }

    if (new_state == APP_STATE_PLAYING) {
        if (audio_is_playing(MUSIC_MENU_LOBBY)) {
            audio_stop_with_fade(MUSIC_MENU_LOBBY, 750, AUDIO_FADE_OUT_BY_VOLUME, NULL);
        }
    }

    if (new_state == APP_STATE_MENU) {
        // En menu, on retire toujours le filtre lobby pour récupérer un son normal.
        audio_set_filter(MUSIC_MENU_LOBBY, AUDIO_FILTER_NONE, 0.0f);

        // Si on revient du jeu, purge la piste menu/lobby potentiellement encore en fade-out.
        if (previous_state == APP_STATE_PLAYING && audio_is_playing(MUSIC_MENU_LOBBY)) {
            audio_stop(MUSIC_MENU_LOBBY);
        }
    }
}

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

    srand(time(NULL));

    /* Assure que le répertoire 'data' existe (non fatal) */
    {
        struct stat st = {0};
        if (stat("data", &st) == -1) {
            if (MKDIR_DATA("data") != 0) {
                fprintf(stderr, "Warning: could not create 'data' directory: %s\n", strerror(errno));
            }
        }
    }

    // Parse command line arguments
    #ifdef _WIN32
        for (int i = 1; i < argc; i++) {
            if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0) && i + 1 < argc) {
                strncpy(ip, argv[++i], sizeof(ip) - 1);
                ip[sizeof(ip) - 1] = '\0';
            } else if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) && i + 1 < argc) {
                port = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Usage: %s [-s server_ip] [-p port]\n", argv[0]);
                return EXIT_FAILURE;
            }
        }
    #else
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
    #endif
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

    AppContext context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    set_game_window_icon(&context);

    // Register App context and cleanup function
    if (define_app_context(resources, &context, destroy_context) != EXIT_SUCCESS) {
        printf("Failed to define App context\n");
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

    load_version(&context);
    printf("Codenames Client Version: %s\n", context.version);

    // Gestion de l'UUID du joueur
    context.player_uuid = NULL;
    {
        FILE* f = fopen("data/uuid", "r");
        if (f) {
            // Le fichier existe, lire l'UUID (deuxième ligne)
            char line[256];
            // Première ligne : "NE PAS MODIFIER CE FICHIER !"
            if (fgets(line, sizeof(line), f)) {
                // Deuxième ligne : l'UUID
                if (fgets(line, sizeof(line), f)) {
                    // Supprimer le '\n' en fin de ligne
                    size_t len = strlen(line);
                    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
                    context.player_uuid = strdup(line);
                    printf("UUID loaded from file: %s\n", context.player_uuid);
                }
            }
            fclose(f);
        }

        if (!context.player_uuid) {
            // Le fichier n'existe pas ou est invalide, demander un UUID au serveur
            char trame[16];
            format_to(trame, sizeof(trame), "%d", MSG_REQUESTUUID);
            send_tcp(context.sock, trame);
            printf("Requesting UUID from server...\n");
        }
    }

    // Initialiser les scènes
    int menu_loading_fails = menu_init(&context);
    if (menu_loading_fails > 0) {
        printf("Failed to load %d menu resource(s)\n", menu_loading_fails);
        menu_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, menu_free);
    SDL_Event e;
    int running = 1;

    typedef int (*SceneInitFn)(AppContext*);
    typedef int (*SceneCleanupFn)(void);

    typedef struct StartupLoadStep {
        const char* name;
        SceneInitFn init;
        SceneCleanupFn cleanup;
    } StartupLoadStep;

    StartupLoadStep startup_steps[] = {
        {"background", init_background, destroy_background},
        {"infos", init_infos, infos_free},
        {"lobby", lobby_init, lobby_free},
        {"cards", init_cards, card_free},
        {"game", game_init, game_free},
    };

    const int startup_step_count = (int)(sizeof(startup_steps) / sizeof(startup_steps[0]));
    int startup_step_index = 0;
    int startup_infos_ready = 0;
    int startup_background_ready = 0;
    int startup_running = 1;
    const Uint32 startup_logo_only_ms = 750U;
    Uint32 startup_sequence_started_ms = SDL_GetTicks();

    menu_set_startup_loading_progress(0.0f);

    while (running && startup_running) {
        if (tick_tcp(&context) != EXIT_SUCCESS) {
            if (context.sock >= 0) {
                close_tcp(context.sock);
                context.sock = -1;
            }
        }

        context.frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0
                && (e.key.keysym.sym == SDLK_F11 || e.key.keysym.sym == SDLK_ESCAPE)) {
                toggle_fullscreen(&context);
            }
            #ifndef _WIN32
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
                    toggle_fullscreen(&context);
                } else if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    Uint32 wflags = SDL_GetWindowFlags(context.window);
                    int is_fs = (wflags & SDL_WINDOW_FULLSCREEN_DESKTOP) || (wflags & SDL_WINDOW_FULLSCREEN);
                    if (is_fs) toggle_fullscreen(&context);
                }
            }
            #endif

            if (startup_infos_ready && menu_is_startup_animation_complete()) {
                infos_handle_event(&context, &e);
            }

            if (startup_step_index >= startup_step_count) {
                if (
                    (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) ||
                    e.type == SDL_FINGERDOWN
                ) {
                    menu_request_startup_skip();
                }
            }
        }

        Uint32 now_ms = SDL_GetTicks();
        if (startup_step_index < startup_step_count && (now_ms - startup_sequence_started_ms) >= startup_logo_only_ms) {
            StartupLoadStep* step = &startup_steps[startup_step_index];
            int loading_fails = step->init(&context);

            if (loading_fails > 0) {
                printf("Failed to load %d %s resource(s)\n", loading_fails, step->name);
                if (step->cleanup) {
                    step->cleanup();
                }
                cleanup_resources(resources);
                return EXIT_FAILURE;
            }

            if (step->cleanup) {
                add_destroy_resource(resources, step->cleanup);
            }

            if (step->init == init_infos) {
                startup_infos_ready = 1;
            }
            if (step->init == init_background) {
                startup_background_ready = 1;
            }

            startup_step_index++;
            menu_set_startup_loading_progress((float)startup_step_index / (float)startup_step_count);

            if (startup_step_index >= startup_step_count) {
                menu_mark_startup_loading_complete();
            }
        }

        float lum_fct = context.global_luminosity;
        context.bg_color = (SDL_Color){90 * lum_fct, 90 * lum_fct, 90 * lum_fct, 255};

        // Rendu du fond animé pendant le chargement du menu
        SDL_SetRenderDrawColor(
            context.renderer,
            context.bg_color.r,
            context.bg_color.g,
            context.bg_color.b,
            context.bg_color.a
        );
        SDL_RenderClear(context.renderer);

        if (startup_background_ready) {
            display_background(&context);
        }
        menu_display(&context);

        if (startup_infos_ready && menu_is_startup_animation_complete()) {
            infos_display(&context);
        }

        SDL_RenderPresent(context.renderer);

        Uint32 frame_end_time = SDL_GetTicks();
        Uint32 frame_elapsed = frame_end_time - context.frame_start_time;
        float frame_duration = 1000.0f / target_fps;
        int frame_delay = (int)(frame_duration - frame_elapsed);
        if (frame_delay > 0) SDL_Delay(frame_delay);

        context.clock++;

        if (startup_step_index >= startup_step_count && menu_is_startup_animation_complete()) {
            startup_running = 0;
        }

        if (auto_close_frames > 0 && context.clock >= auto_close_frames) {
            running = 0;
        }
    }

    if (!running) {
        printf("Exiting...\n");
        cleanup_resources(resources);

        if (context.restart_requested) {
            if (relaunch_game_process(argc, argv) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }

    AppState previous_app_state = context.app_state;

    while (running) {
        if (tick_tcp(&context) != EXIT_SUCCESS) {
            if (context.sock >= 0) {
                close_tcp(context.sock);
                context.sock = -1;
            }
        }

        // Enregistrer le début de la frame
        context.frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0
                && (e.key.keysym.sym == SDLK_F11 || e.key.keysym.sym == SDLK_ESCAPE)) {
                toggle_fullscreen(&context);
            }
            #ifndef _WIN32
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
                    toggle_fullscreen(&context);
                } else if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    Uint32 wflags = SDL_GetWindowFlags(context.window);
                    int is_fs = (wflags & SDL_WINDOW_FULLSCREEN_DESKTOP) || (wflags & SDL_WINDOW_FULLSCREEN);
                    if (is_fs) toggle_fullscreen(&context);
                }
            }
            #endif
            /* Le bandeau infos (crossfaders volume) est toujours actif */
            infos_handle_event(&context, &e);

            /* Fond interactif (clic sur un symbole) */
            if (context.app_state == APP_STATE_MENU || context.app_state == APP_STATE_LOBBY || context.app_state == APP_STATE_PLAYING)
                background_handle_event(&context, &e);

            // Déléguer l'événement selon l'état courant
            switch (context.app_state) {
                ButtonReturn btn_ret;

                case APP_STATE_MENU:
                    btn_ret = menu_handle_event(&context, &e);
                    if (btn_ret == BTN_MENU_QUIT) {
                        running = 0;
                    } break;
                case APP_STATE_LOBBY: lobby_handle_event(&context, &e); break;
                case APP_STATE_PLAYING:
                    game_handle_event(&context, &e);
                    if (context.app_state == APP_STATE_PLAYING) {
                        cards_handle_event(&context, &e);
                    }
                    break;
                case APP_STATE_PAUSED: break;
            }
        }

        handle_app_state_audio_transition(&context, previous_app_state, context.app_state);
        previous_app_state = context.app_state;

        // Pré Rendu - Utiliser la couleur du background
        SDL_SetRenderDrawColor(context.renderer, context.bg_color.r, context.bg_color.g, context.bg_color.b, context.bg_color.a);
        SDL_RenderClear(context.renderer);

        float lum_fct = context.global_luminosity;
        // Rendu et logique d'affichage 
        switch (context.app_state) {
            case APP_STATE_MENU:
                context.bg_color = (SDL_Color){90*lum_fct, 90*lum_fct, 90*lum_fct, 255}; // Gris par défaut

                if (menu_should_render_background()) {
                    display_background(&context);
                }
                menu_display(&context);
                break;
            case APP_STATE_LOBBY:
                context.bg_color = (SDL_Color){90*lum_fct, 90*lum_fct, 90*lum_fct, 255};  // Gris par défaut

                display_background(&context);
                lobby_display(&context);
                break;
            case APP_STATE_PLAYING:
                if (context.lobby->game->state == GAMESTATE_TURN_RED_SPY || context.lobby->game->state == GAMESTATE_TURN_RED_AGENT) {
                    context.bg_color = (SDL_Color){120*lum_fct, 45*lum_fct, 45*lum_fct, 255}; // Rouge sombre pour l'équipe rouge
                } else {
                    context.bg_color = (SDL_Color){55*lum_fct, 55*lum_fct, 120*lum_fct, 255}; // Bleu sombre pour l'équipe bleue
                }
                
                display_background(&context);
                game_display(&context);
                break;
            case APP_STATE_PAUSED:
                // TODO: Implement paused state rendering
                break;
        }

        // Onglet d'infos
        infos_display(&context);

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

    if (context.restart_requested) {
        if (relaunch_game_process(argc, argv) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}