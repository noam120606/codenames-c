#include "../../client/lib/all.h"

/* Fichier de test pour le fichier game.c du client */

int main(int argc, char* argv[]) {

    const float target_fps = 60.0f;
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

    Resources* resources = init_resources();
    if (!resources) {
        printf("Failed to initialize resources\n");
        return EXIT_FAILURE;
    }

    /* se placer dans le dossier client pour que les chemins relatifs (assets, audio...) fonctionnent */
    if (chdir("../../client") != 0) {
        perror("chdir");
        /* on continue quand même : si le chdir échoue, init_sdl échouera proprement */
    }

    AppContext context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }
    
    if (define_app_context(resources, &context, destroy_context) != EXIT_SUCCESS) {
        printf("Failed to define SDL context\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    int game_loading_fails = game_init(&context);
    if (game_loading_fails > 0) {
        printf("Failed to load %d game resource(s)\n", game_loading_fails);
        game_free();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, game_free);

    int bg_loading_fails = init_background(&context);
    if (bg_loading_fails > 0) {
        printf("Failed to load %d background resource(s)\n", bg_loading_fails);
        destroy_background();
        cleanup_resources(resources);
        return EXIT_FAILURE;
    } else add_destroy_resource(resources, destroy_background);

    SDL_Event e;
    int running = 1;

    // Initialisation des valeurs de context pour les tests de l'affichage des cartes
    context.player_role = ROLE_SPY;

    int gender = rand() % 2; /* 0 = homme, 1 = femme */
    int i = 0;

    /* Ensure a Game structure and its words buffer exist before writing to them.
     * In the real client these are created when a MSG_STARTGAME message is handled.
     */
    context.lobby->game = malloc(sizeof(Game));
    if (!context.lobby->game) {
        printf("Failed to allocate game for test\n");
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }
    context.lobby->game->nb_words = NB_WORDS;
    context.lobby->game->state = GAMESTATE_WAITING;
    context.lobby->game->words = malloc(sizeof(Word) * context.lobby->game->nb_words);
    if (!context.lobby->game->words) {
        printf("Failed to allocate game words for test\n");
        free(context.lobby->game);
        context.lobby->game = NULL;
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }

    /* Initialize words in black card */
    strncpy(context.lobby->game->words[0].word, "Test", sizeof(context.lobby->game->words[0].word) - 1);
    context.lobby->game->words[0].word[sizeof(context.lobby->game->words[0].word) - 1] = '\0';
    context.lobby->game->words[0].team = TEAM_BLACK;
    context.lobby->game->words[0].revealed = 0;
    context.lobby->game->words[0].gender = (gender++ % 2);

    /* Initialize words in red cards */
    for (i = 1; i < 9; i++) {
        strncpy(context.lobby->game->words[i].word, "Test", sizeof(context.lobby->game->words[i].word) - 1);
        context.lobby->game->words[i].word[sizeof(context.lobby->game->words[i].word) - 1] = '\0';
        context.lobby->game->words[i].team = TEAM_RED;
        context.lobby->game->words[i].revealed = 0;
        context.lobby->game->words[i].gender = (gender++) % 2; 
    }

    /* Initialize words in blue cards */
    for (i = 9; i < 18; i++) {
        strncpy(context.lobby->game->words[i].word, "Test", sizeof(context.lobby->game->words[i].word) - 1);
        context.lobby->game->words[i].word[sizeof(context.lobby->game->words[i].word) - 1] = '\0';
        context.lobby->game->words[i].team = TEAM_BLUE;
        context.lobby->game->words[i].revealed = 0;
        context.lobby->game->words[i].gender = (gender++) % 2; 
    }

    /* Initialize words in neutral cards */
    for (i = 18; i < 25; i++) {
        strncpy(context.lobby->game->words[i].word, "Test", sizeof(context.lobby->game->words[i].word) - 1);
        context.lobby->game->words[i].word[sizeof(context.lobby->game->words[i].word) - 1] = '\0';
        context.lobby->game->words[i].team = TEAM_NONE;
        context.lobby->game->words[i].revealed = 0;
        context.lobby->game->words[i].gender = (gender++) % 2; 
    }


    while (running && tick_tcp(&context) == EXIT_SUCCESS) {
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
            game_handle_event(&context, &e);
        }
        
        SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
        SDL_RenderClear(context.renderer);
        game_render_cards(&context);

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
    
    cleanup_resources(resources);

    return EXIT_SUCCESS;
}
