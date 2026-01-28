#ifndef TCP_H
#define TCP_H

#include <netinet/in.h>
#include "../lib/game_manager.h"

#define TCP_MAX_CLIENTS 128
#define TCP_BUFFER_SIZE 1024

typedef struct {
    int id;
    int socket;
    struct sockaddr_in addr;
} TcpClient;

typedef struct {
    int server_socket;
    int port;
    TcpClient clients[TCP_MAX_CLIENTS];
    int next_client_id;
} TcpServer;

/* Initialisation / destruction */
TcpServer* tcp_server_create(int port);
void tcp_server_destroy(TcpServer* server);

/* Tick à appeler dans la boucle principale */
void tcp_server_tick(TcpServer* server, GameManager* game_manager);

/* Communication */
int tcp_send_to_client(TcpServer* server, int client_id, const char* message, GameManager* game_manager);

/* Callbacks (à override si besoin) */
void tcp_on_client_connect(TcpServer* server, TcpClient* client, GameManager* game_manager);
void tcp_on_client_disconnect(TcpServer* server, TcpClient* client);
void tcp_on_client_message(TcpServer* server, TcpClient* client, const char* message);

#endif