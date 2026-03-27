#include "../lib/all.h"

static void set_nonblocking(int socket) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket((SOCKET)socket, FIONBIO, &mode);
#else
    fcntl(socket, F_SETFL, O_NONBLOCK);
#endif
}

TcpServer* tcp_server_create(int port) {
    TcpServer* server = calloc(1, sizeof(TcpServer));
    if (!server) return NULL;

    server->port = port;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup");
        free(server);
        return NULL;
    }
#endif

    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket < 0) {
        perror("socket");
#ifdef _WIN32
        WSACleanup();
#endif
        free(server);
        return NULL;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server->server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        CLOSESOCKET(server->server_socket);
    #ifdef _WIN32
        WSACleanup();
    #endif
        free(server);
        return NULL;
    }

    if (listen(server->server_socket, 10) < 0) {
        perror("listen");
        CLOSESOCKET(server->server_socket);
    #ifdef _WIN32
        WSACleanup();
    #endif
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
            CLOSESOCKET(server->clients[i].socket);
    }

    CLOSESOCKET(server->server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    free(server);
}

static void add_client(Codenames* codenames, int client_socket, struct sockaddr_in addr) {
    TcpServer* server = codenames->tcp;
    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (server->clients[i].socket == 0) {
            server->clients[i].socket = client_socket;
            server->clients[i].addr = addr;
            server->clients[i].id = i;

            set_nonblocking(client_socket);
            tcp_on_client_connect(codenames, &server->clients[i]);
            return;
        }
    }

    CLOSESOCKET(client_socket);
}

static void remove_client(Codenames* codenames, TcpClient* client) {
    tcp_on_client_disconnect(codenames, client);
    CLOSESOCKET(client->socket);
    memset(client, 0, sizeof(TcpClient));
}

void tcp_disconnect(Codenames* codenames, TcpClient* client) {
    remove_client(codenames, client);
}

void tcp_server_tick(Codenames* codenames) {
    TcpServer* server = codenames->tcp;

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
            add_client(codenames, client_socket, addr);
        }
    }

    char buffer[TCP_BUFFER_SIZE];

    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        TcpClient* client = &server->clients[i];
        if (client->socket > 0 && FD_ISSET(client->socket, &readfds)) {
            int bytes = recv(client->socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                remove_client(codenames, client);
            } else {
                buffer[bytes] = '\0';
                tcp_on_client_message(codenames, client, buffer);
            }
        }
    }
}

// Fonctions utilisables 

int tcp_send_to_client(Codenames* codenames, int client_id, const char* message) {
    for (int i = 0; i < TCP_MAX_CLIENTS; i++) {
        if (codenames->tcp->clients[i].id == client_id) {
            size_t len = strlen(message);
            char* msg_with_newline = malloc(len + 2);
            if (!msg_with_newline) return -1;
            memcpy(msg_with_newline, message, len);
            msg_with_newline[len] = '\n';
            msg_with_newline[len + 1] = '\0';
            int result = send(codenames->tcp->clients[i].socket, msg_with_newline, len + 1, 0);
            free(msg_with_newline);
            return result;
        }
    }
    return -1;
}

void tcp_on_client_connect(Codenames* codenames, TcpClient* client) {
    // printf("Client connected: ID=%d IP=%s\n", client->id, inet_ntoa(client->addr.sin_addr));
    // tcp_send_to_client(codenames, client->id, "Welcome to the server!\n");
}

void tcp_on_client_disconnect(Codenames* codenames, TcpClient* client) {
    // printf("Client disconnected: ID=%d\n", client->id);
    on_leave(codenames, client);
}

void tcp_on_client_message(Codenames* codenames, TcpClient* client, char* message) {

    // Verifie que le message est bien envoyé par le client et pas un navigateur ou autre
    if (!starts_with(message, "CODENAMES ")) {
        tcp_send_to_client(codenames, client->id, "ERROR: Methode de connexion invalide\n");
        remove_client(codenames, client);
        return;
    }

    char* payload = message + strlen("CODENAMES ");

    on_message(codenames, client, payload);
}
