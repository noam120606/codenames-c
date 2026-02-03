#include <stdlib.h>
#include <string.h>
#include "../lib/user.h"

User* create_user(int id, const char* name, int socket_fd) {
    User* user = malloc(sizeof(User));
    if (!user) return NULL;

    user->id = id;
    user->name = strdup(name);
    user->socket_fd = socket_fd;
    user->role = 0;
    user->team = 0;

    return user;
}

void destroy_user(User* user) {
    if (user) {
        free(user->name);
        free(user);
    }
}
