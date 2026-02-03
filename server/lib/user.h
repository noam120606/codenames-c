#ifndef USER_H
#define USER_H

#define ROLE_ESPION 1
#define ROLE_AGENT 2

typedef struct {
    int id;
    char* name;

    int socket_fd;

    int role;
    int team; // TEAM (game.h)
} User;

// Fonctions
User* create_user(int id, const char* name, int socket_fd);
void destroy_user(User* user);

#endif // USER_H