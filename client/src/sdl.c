#include "../lib/all.h"

SDL_Context init_sdl() {
    SDL_Context context = {0}; // Initialisation sécurisée à zéro

    // Log dimensions
    printf("WIN_WIDTH = %d, WIN_HEIGHT = %d\n", WIN_WIDTH, WIN_HEIGHT);

    // Initialize SDL
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return context;
    }

    // Create SDL window
    printf("Creating SDL window...\n");
    // SDL_Window* win = SDL_CreateWindow("Codenames", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Window* win = SDL_CreateWindow("Codenames", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return context;
    }

    context.window = win;

    // Create SDL renderer
    printf("Creating SDL renderer...\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return context;
    }

    context.renderer = renderer;

    // Initialize IMG
    printf("Initializing IMG...\n");
    if (!(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG)) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return context;
    }

    // Initialize TTF
    printf("Initializing TTF...\n");
    if(TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return context;
    }
    
    context.clock = 0;
    context.fps = 0.0f;
    context.sock = 0;
    context.frame_start_time = 0;
    
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
    printf("Image '%s' chargée : %dx%d\n", path, tex_w, tex_h);
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

void destroy_context(SDL_Context context) {
    if (context.renderer) {
        SDL_DestroyRenderer(context.renderer);
    }
    if (context.window) {
        SDL_DestroyWindow(context.window);
    }
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}