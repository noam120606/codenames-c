#include "../lib/all.h"


Resources* init_resources() {
    Resources* res = (Resources*)malloc(sizeof(Resources));
    res->nb_resources = 0;
    return res;
}

int define_sdl_context(Resources* res, SDL_Context* context, DestroyContext destroy_context) {
    if (!res || !context) return EXIT_FAILURE;
    res->sdl_context = context;
    res->destroy_context = destroy_context;
    return EXIT_SUCCESS;
}

int add_destroy_resource(Resources* res, DestroyFunction destroy) {
    if (res->nb_resources < MAX_RESOURCES) {
        res->destroy_functions[res->nb_resources++] = destroy;
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void cleanup_resources(Resources* res) {
    for (int i = 0; i < res->nb_resources; i++) {
        if (res->destroy_functions[i]) {
            res->destroy_functions[i]();
        }
    }
    if (res->destroy_context) {
        res->destroy_context(res->sdl_context);
    }
    free(res);
}