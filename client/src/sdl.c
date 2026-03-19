#include "../lib/all.h"

static int is_truthy_env(const char* value) {
    return value && strcmp(value, "0") != 0 && value[0] != '\0';
}

static int is_running_under_valgrind(void) {
    const char* ld_preload = getenv("LD_PRELOAD");
    return getenv("VALGRIND_LIB") || (ld_preload && strstr(ld_preload, "valgrind"));
}

/* Détermine si l'audio factice doit être utilisé.
 * Peut être activé via la variable d'environnement CODENAMES_AUDIO_DUMMY ou automatiquement
 * si le programme est exécuté sous Valgrind. */
static int should_use_dummy_audio(void) {
    return is_truthy_env(getenv("CODENAMES_AUDIO_DUMMY")) || is_running_under_valgrind();
}

/* Configure des hints et variables d'environnement SDL pour améliorer l'expérience sous Valgrind
   et éviter les popups d'erreur liés à l'IME ou D-Bus. */
static void configure_diagnostic_sdl_environment(void) {
    if (!is_running_under_valgrind()) {
        return;
    }

    SDL_SetHint(SDL_HINT_SHUTDOWN_DBUS_ON_QUIT, "1");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "0");
    SDL_setenv("SDL_IM_MODULE", "none", 1);
    SDL_setenv("XMODIFIERS", "@im=none", 1);
}

static Uint32 get_renderer_flags(void) {
    if (is_truthy_env(getenv("CODENAMES_RENDER_SOFTWARE")) || is_running_under_valgrind()) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
        SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");
        return SDL_RENDERER_SOFTWARE;
    }
    return SDL_RENDERER_ACCELERATED;
}

SDL_Context init_sdl() {
    SDL_Context context = {0}; // Initialisation sécurisée à zéro

    // Log dimensions
    printf("WIN_WIDTH = %d, WIN_HEIGHT = %d\n", WIN_WIDTH, WIN_HEIGHT);

    configure_diagnostic_sdl_environment();

    if (should_use_dummy_audio()) {
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    }

    // Initialize SDL
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return context;
    }

    // Create SDL window
    printf("Creating SDL window...\n");
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_Window* win = SDL_CreateWindow("Codenames", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, window_flags);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return context;
    }

    SDL_SetWindowMinimumSize(win, WIN_MIN_WIDTH, WIN_MIN_HEIGHT);

    context.window = win;

    // Create SDL renderer
    printf("Creating SDL renderer...\n");
    Uint32 renderer_flags = get_renderer_flags();
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, renderer_flags);
    if (renderer == NULL && renderer_flags != SDL_RENDERER_SOFTWARE) {
        printf("SDL_CreateRenderer accelerated failed, fallback to software: %s\n", SDL_GetError());
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
        renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return context;
    }

    context.renderer = renderer;
    SDL_RenderSetLogicalSize(renderer, WIN_WIDTH, WIN_HEIGHT);

    // Initialize IMG
    printf("Initializing IMG...\n");
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int img_initialized = IMG_Init(img_flags);
    if ((img_initialized & img_flags) != img_flags) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return context;
    }

    // Initialize TTF
    printf("Initializing TTF...\n");
    if(TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return context;
    }

    // Initialize Mixer (skip if audio disabled/dummy)
    printf("Initializing Mixer...\n");
    if (is_truthy_env(getenv("CODENAMES_AUDIO_DISABLED")) || is_truthy_env(getenv("CODENAMES_AUDIO_DUMMY"))) {
        printf("Audio initialization skipped (CODENAMES_AUDIO_DISABLED or CODENAMES_AUDIO_DUMMY set)\n");
    } else {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
            printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
            TTF_Quit();
            IMG_Quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(win);
            SDL_Quit();
            return context;
        }
    }

    context.clock = 0;
    context.fps = 0.0f;
    context.sock = 0;
    context.lobby = (Lobby*)malloc(sizeof(Lobby));
    if (!context.lobby) {
        printf("Failed to allocate memory for lobby\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return context;
    }
    context.lobby->id = -1;
    context.frame_start_time = 0;
    context.ping_ms = -1;
    context.app_state = APP_STATE_MENU;
    context.music_volume = MIX_MAX_VOLUME;
    context.sound_effects_volume = MIX_MAX_VOLUME;

    if (audio_init() != EXIT_SUCCESS) {
        printf("audio_init Error\n");
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        context.renderer = NULL;
        context.window = NULL;
        return context;
    }
    
    printf("All initialized successfully!\n");
    
    return context;
}

SDL_Texture* load_image(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* image = IMG_Load(path);
    if (!image) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture) {
        printf("Erreur de création de la texture : %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        return NULL;
    }
    int tex_w = 0, tex_h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h);
    if (tex_w == 0 || tex_h == 0) {
        printf("Image '%s' failed to load : %dx%d\n", path, tex_w, tex_h);
    }
    SDL_FreeSurface(image);
    return texture;
}

int display_image(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, float size_factor, double angle, SDL_RendererFlip flip, float ratio, Uint8 opacity) {
    if (!renderer || !texture) {
        return EXIT_FAILURE;
    }

    int og_w = 0, og_h = 0;
    if (SDL_QueryTexture(texture, NULL, NULL, &og_w, &og_h) != 0) {
        printf("SDL_QueryTexture Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Rect dstrect;
    dstrect.x = (WIN_WIDTH - og_w*size_factor)/2 + x;
    dstrect.y = (WIN_HEIGHT - og_h*size_factor)/2 - y; /* Inversion de l'axe y (seulement)*/

    int final_w = og_w;
    int final_h = og_h;

    /* Calcul des ratios si altérés */
    if (ratio != 1) {
        final_w = og_w * ratio;
        dstrect.x = (WIN_WIDTH - final_w*size_factor)/2 + x;
    }
    dstrect.w = (ratio != 1) ? final_w*size_factor : og_w*size_factor;
    dstrect.h = (ratio != 1) ? final_h*size_factor : og_h*size_factor;

    /* Point de pivot au centre de l'image */
    SDL_Point center = {dstrect.w / 2, dstrect.h / 2};

    /* Appliquer l'opacité */
    SDL_SetTextureAlphaMod(texture, opacity);

    /* Utiliser SDL_RenderCopyEx pour supporter la rotation et le flip */
    SDL_RenderCopyEx(renderer, texture, NULL, &dstrect, angle, &center, flip);

    return EXIT_SUCCESS;
}

void free_image(SDL_Texture* texture) {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

void toggle_fullscreen(SDL_Context* context) {
    if (!context || !context->window) return;
    Uint32 flags = SDL_GetWindowFlags(context->window);
    int is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) || (flags & SDL_WINDOW_FULLSCREEN);
    SDL_SetWindowFullscreen(context->window, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (is_fullscreen) {
        // Recentrer la fenêtre après être sorti du plein écran
        SDL_SetWindowPosition(context->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

int destroy_context(SDL_Context* context) {
    if (!context) {
        return EXIT_FAILURE;
    }

    if (context->renderer) {
        SDL_DestroyRenderer(context->renderer);
        context->renderer = NULL;
    }
    if (context->window) {
        SDL_DestroyWindow(context->window);
        context->window = NULL;
    }
    if (context->sock > 0) {
        close_tcp(context->sock);
        context->sock = -1;
    }
    if (context->player_uuid) {
        free(context->player_uuid);
        context->player_uuid = NULL;
    }
    audio_cleanup();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}