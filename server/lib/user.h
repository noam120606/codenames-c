#ifndef USER_H
#define USER_H

#include "game.h"

/** Rôles possibles d'un utilisateur.
 * @param ROLE_ESPION donne des indices.
 * @param ROLE_AGENT devine les mots.
 */
typedef enum UserRole {
    ROLE_ESPION,
    ROLE_AGENT
} UserRole;

/** Représente un joueur connecté.
 * @param id identifiant unique de l'utilisateur.
 * @param name nom du joueur.
 * @param socket_fd id de socket associée au joueur.
 * @param role rôle (ROLE_ESPION ou ROLE_AGENT).
 * @param team équipe (TEAM_*).
 */
typedef struct {
    int id;
    char* name;
    int socket_fd;
    UserRole role;
    Team team;
} User;

/* Fonctions */


/** Crée un utilisateur.
 * @param id Identifiant de l'utilisateur.
 * @param name Nom du joueur (copié en interne).
 * @param socket_fd Socket associée.
 * @return Pointeur vers le User créé, ou NULL en cas d'erreur.
 */
User* create_user(int id, const char* name, int socket_fd);

/** Détruit un utilisateur et libère ses ressources.
 * @param user Utilisateur à détruire.
 */
void destroy_user(User* user);

#endif // USER_H