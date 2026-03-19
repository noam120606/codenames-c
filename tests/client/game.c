#include "../../client/lib/all.h"

/* Fichier de test pour le fichier game.c du client */

int main(int argc, char* argv[]) {

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

    SDL_Context context = init_sdl();
    if (context.window == NULL || context.renderer == NULL) {
        printf("Failed to initialize SDL\n");
        destroy_context(&context);
        cleanup_resources(resources);
        return EXIT_FAILURE;
    }
    
    if (define_sdl_context(resources, &context, destroy_context) != EXIT_SUCCESS) {
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

    SDL_Event e;
    int running = 1;

    // Initialisation des valeurs de context pour les tests de l'affichage des cartes
    context.player_role = ROLE_SPY;

    int gender = 0, i = 0;

    context.cards[i] = malloc(sizeof(Card));
    context.cards[i]->revealed = 0;
    context.cards[i]->gender = (gender++)%2==0 ? 0 : 1;
    context.cards[i]->team = TEAM_BLACK;

    for (i = 1; i < 9; i++) {
        context.cards[i] = malloc(sizeof(Card));
        context.cards[i]->revealed = 0;
        context.cards[i]->gender = (gender++)%2==0 ? 0 : 1;
        context.cards[i]->team = TEAM_RED;
    }
    for (i = 9; i < 17; i++) {
        context.cards[i] = malloc(sizeof(Card));
        context.cards[i]->revealed = 0;
        context.cards[i]->gender = (gender++)%2==0 ? 0 : 1;
        context.cards[i]->team = TEAM_BLUE;
    }
    for (i = 17; i < 25; i++) {
        context.cards[i] = malloc(sizeof(Card));
        context.cards[i]->revealed = 0;
        context.cards[i]->gender = (gender++)%2==0 ? 0 : 1;
        context.cards[i]->team = TEAM_NONE;
    }


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
            game_handle_event(&context, &e);
        }

        SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
        SDL_RenderClear(context.renderer);

        game_display(&context);

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