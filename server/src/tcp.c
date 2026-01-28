#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "../lib/tcp.h"

static void set_nonblocking(int socket) {
    fcntl(socket, F_SETFL, O_NONBLOCK);
}

TcpServer* tcp_server_create(int port) {
    TcpServer* server = calloc(1, sizeof(TcpServer));
    if (!server) return NULL;

    server->port = port;
    server->next_client_id = 1;

    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket < 0) {
        perror("socket");
        free(server);
        return NULL;
    }

    int opt = 1;
    setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server->server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server->server_socket);
        free(server);
        return NULL;
    }

    if (listen(server->server_socket, 10) < 0) {
        perror("listen");
        close(server->server_socket);
        free(server);
        return NULL;
    }

    set_nonblocking(server->server_socket);

    printf("TCP server listening on port %d\n", port);
    return server;
}

void tcp_server_destroy(TcpServer* server) {
    if (!server) return;

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (server->clients[i].socket > 0)
            close(server->clients[i].socket);
    }

    close(server->server_socket);
    free(server);
}

static void add_client(TcpServer* server, int client_socket, struct sockaddr_in addr) {
    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (server->clients[i].socket == 0) {
            server->clients[i].socket = client_socket;
            server->clients[i].addr = addr;
            server->clients[i].id = server->next_client_id++;

            set_nonblocking(client_socket);
            tcp_on_client_connect(server, &server->clients[i]);
            return;
        }
    }

    close(client_socket);
}

static void remove_client(TcpServer* server, TcpClient* client) {
    tcp_on_client_disconnect(server, client);
    close(client->socket);
    memset(client, 0, sizeof(TcpClient));
}

void tcp_server_tick(TcpServer* server) {
    fd_set readfds;
    FD_ZERO(&readfds);

    int max_fd = server->server_socket;
    FD_SET(server->server_socket, &readfds);

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        int sock = server->clients[i].socket;
        if (sock > 0) {
            FD_SET(sock, &readfds);
            if (sock > max_fd) max_fd = sock;
        }
    }

    struct timeval tv = {0, 0}; // non bloquant
    int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (activity <= 0) return;

    if (FD_ISSET(server->server_socket, &readfds)) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int client_socket = accept(server->server_socket, (struct sockaddr*)&addr, &len);
        if (client_socket >= 0) {
            add_client(server, client_socket, addr);
        }
    }

    char buffer[TCP_BUFFER_SIZE];

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        TcpClient* client = &server->clients[i];
        if (client->socket > 0 && FD_ISSET(client->socket, &readfds)) {
            int bytes = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                remove_client(server, client);
            } else {
                buffer[bytes] = '\0';
                tcp_on_client_message(server, client, buffer);
            }
        }
    }
}

int tcp_send_to_client(TcpServer* server, int client_id, const char* message) {
    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (server->clients[i].id == client_id) {
            return send(server->clients[i].socket, message, strlen(message), 0);
        }
    }
    return -1;
}

/* Callbacks par défaut */

void tcp_on_client_connect(TcpServer* server, TcpClient* client) {
    printf("Client connected: ID=%d IP=%s\n",
           client->id, inet_ntoa(client->addr.sin_addr));
    
    tcp_send_to_client(server, client->id, "Welcome to the server!\n");
}

void tcp_on_client_disconnect(TcpServer* server, TcpClient* client) {
    printf("Client disconnected: ID=%d\n", client->id);
}

void tcp_on_client_message(TcpServer* server, TcpClient* client, const char* message) {
    printf("Client %d says: %s\n", client->id, message);
}
