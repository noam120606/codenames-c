#ifndef TCP_H
#define TCP_H

#include "sdl.h"

/**
 *  Initialise une connexion TCP vers le serveur.
 * @param server_ip Adresse IP du serveur.
 * @param port Port du serveur.
 * @return Descripteur de socket, ou valeur négative en cas d'erreur.
 */
int init_tcp(const char* server_ip, int port);

/**
 * Traite les événements réseau côté client (réception, etc.).
 * @param sock Descripteur de socket.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int tick_tcp(SDL_Context* context, int sock);

/**
 * Envoie un message au serveur.
 * @param sock Descripteur de socket.
 * @param message Message à envoyer.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int send_tcp(int sock, const char* message);

/**
 * Ferme la connexion TCP.
 * @param sock Descripteur de socket.
 * @return `EXIT_SUCCESS` en cas de succès, `EXIT_FAILURE` en cas d'erreur.
 */
int close_tcp(int sock);

#endif // TCP_H