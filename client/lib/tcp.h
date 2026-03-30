/**
 * @file tcp.h
 * @brief Communication TCP avec le serveur de jeu.
 */

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
int tick_tcp(AppContext* context);

/**
 * Lit le ping TCP (RTT) depuis la socket, en millisecondes.
 * @param sock Descripteur de socket.
 * @return Ping en ms, ou -1 si indisponible.
 */
int get_tcp_ping_ms(int sock);

/**
 * Indique si la socket est connectée à un serveur local (127.0.0.0/8).
 * @param sock Descripteur de socket.
 * @return 1 si local, 0 sinon.
 */
int is_tcp_local_server(int sock);

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