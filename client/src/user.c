#include "../lib/all.h"

User* create_user(int id, const char* name, UserRole role, Team team) {
    User* user = malloc(sizeof(User));
    if (!user) return NULL;

    user->id = id;
    user->name = strdup(name);
    user->role = role;
    user->team = team;

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
