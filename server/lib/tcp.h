#ifndef TCP_H
#define TCP_H

#include <netinet/in.h>

#define TCP_MAX_CLIENTS 128
#define TCP_BUFFER_SIZE 1024

/** Représente un client TCP.
 * @param id identifiant interne.
 * @param socket descripteur de socket.
 * @param addr adresse réseau du client (struct sockaddr_in).
 */
typedef struct {
    int id;
    int socket;
    struct sockaddr_in addr;
} TcpClient;

/** Représente le serveur TCP.
 * @param server_socket socket d'écoute.
 * @param port port écouté.
 * @param clients tableau de clients connectés.
 * @param next_client_id générateur d'identifiants.
 */
typedef struct {
    int server_socket;
    int port;
    TcpClient clients[TCP_MAX_CLIENTS];
    int next_client_id;
} TcpServer;

/* Initialisation / destruction */

/** 
 * Crée et initialise un serveur TCP.
 * @param port Port d'écoute.
 * @return Pointeur vers TcpServer, ou NULL en cas d'erreur.
 */
TcpServer* tcp_server_create(int port);

/** 
 * Détruit un serveur TCP et libère ses ressources.
 * @param server Serveur à détruire.
 */
void tcp_server_destroy(TcpServer* server);

#include "../lib/codenames.h"

/** 
 * Tick à appeler dans la boucle principale du serveur pour gérer les événements réseau.
 * @param codenames Contexte principal du serveur.
 */
void tcp_server_tick(Codenames* codenames);

/* Communication */

/** 
 * Envoie un message à un client spécifique.
 * @param codenames Contexte principal du serveur.
 * @param client_id Identifiant du client.
 * @param message Message à envoyer.
 * @return 0 en cas de succès, valeur négative en cas d'erreur.
 */
int tcp_send_to_client(Codenames* codenames, int client_id, const char* message);

/* Callbacks (à override si besoin) */

void tcp_on_client_connect(Codenames* codenames, TcpClient* client);
void tcp_on_client_disconnect(Codenames* codenames, TcpClient* client);
void tcp_on_client_message(Codenames* codenames, TcpClient* client, char* message);

#endif // TCP_H