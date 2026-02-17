#include "../lib/all.h"

User* create_user(int id, const char* name, int socket_fd) {
    User* user = malloc(sizeof(User));
    if (!user) return NULL;

    user->id = id;
    user->name = strdup(name);
    user->socket_fd = socket_fd;
    user->role = ROLE_AGENT;
    user->team = TEAM_NEUTRAL;

    if (strcmp(user->name, "NONE") == 0) {
        free(user->name);
        user->name = getRandomUsername();
        if (user->name == NULL) {
            free(user);
            return NULL;
        }
    }

    return user;
}

void destroy_user(User* user) {
    if (user) {
        free(user->name);
        free(user);
    }
}
