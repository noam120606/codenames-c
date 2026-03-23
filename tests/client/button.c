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

    /* Initialisation des boutons pour les tests */
    #define NB_BUTTONS 4
    Button* buttons[NB_BUTTONS] = { NULL, NULL, NULL, NULL };


    // Chargement boutons
    ButtonConfig* cfg_button_basic = button_config_init();
    if (cfg_button_basic) {
        cfg_button_basic->x = -500;
        cfg_button_basic->y = 300;
        cfg_button_basic->font_path = FONT_LARABIE;
        cfg_button_basic->color     = COL_WHITE;
        cfg_button_basic->text      = "Test basic";
        buttons[0] = button_create(context.renderer, 0, cfg_button_basic);
        free(cfg_button_basic);
    }
    ButtonConfig* cfg_button_red = button_config_init();
    if (cfg_button_red) {
        cfg_button_red->y = 300;
        cfg_button_red->font_path = FONT_LARABIE;
        cfg_button_red->color     = COL_RED;
        cfg_button_red->text      = "Test rouge";
        buttons[1] = button_create(context.renderer, 0, cfg_button_red);
        free(cfg_button_red);
    }
    ButtonConfig* cfg_button_blue = button_config_init();
    if (cfg_button_blue) {
        cfg_button_blue->x = 500;
        cfg_button_blue->y = 300;
        cfg_button_blue->font_path = FONT_LARABIE;
        cfg_button_blue->color     = COL_BLUE;
        cfg_button_blue->text      = "Test bleu";
        buttons[2] = button_create(context.renderer, 0, cfg_button_blue);
        free(cfg_button_blue);
    }
    ButtonConfig* cfg_button_big = button_config_init();
    if (cfg_button_big) {
        cfg_button_big->y = -100;
        cfg_button_big->w = 1500;
        cfg_button_big->h = 500;
        cfg_button_big->font_path = FONT_LARABIE;
        cfg_button_big->color     = COL_WHITE;
        cfg_button_big->text      = "Test grand";
        buttons[3] = button_create(context.renderer, 0, cfg_button_big);
        free(cfg_button_big);
    }

    SDL_Event e;
    int running = 1;

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
            for (int i = 0; i < NB_BUTTONS; i++) {
                if (buttons[i]) button_handle_event(&context, buttons[i], &e);
            }
        }

        SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
        SDL_RenderClear(context.renderer);

        for (int i = 0; i < NB_BUTTONS; i++) {
            if (buttons[i]) button_render(context.renderer, buttons[i]);
        }

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

    for (int i = 0; i < NB_BUTTONS; i++) {
        if (buttons[i]) button_destroy(buttons[i]);
    }

    return EXIT_SUCCESS;
}
