#include "../lib/all.h"


Resources* init_resources() {
    Resources* res = (Resources*)calloc(1, sizeof(Resources));
    return res;
}

int define_app_context(Resources* res, AppContext* context, DestroyContext destroy_context) {
    if (!res || !context) return EXIT_FAILURE;
    res->context = context;
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
    if (!res) {
        return;
    }

    for (int i = 0; i < res->nb_resources; i++) {
        if (res->destroy_functions[i]) {
            res->destroy_functions[i]();
        }
    }
    if (res->destroy_context) {
        res->destroy_context(res->context);
    }
    free(res);
}