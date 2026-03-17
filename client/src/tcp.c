#include "../lib/all.h"
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

int init_tcp(const char* server_ip, int port) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

int tick_tcp(SDL_Context* context, int sock) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

    struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};
    int result = select(maxfd + 1, &readfds, NULL, NULL, &timeout); // Timeout pour avoir une boucle non bloquante
    
    if (result < 0) {
        perror("select");
        return -1;
    }
    
    if (result == 0) {
        return EXIT_SUCCESS; // No data available
    }

    /* Message venant du serveur */
    if (FD_ISSET(sock, &readfds)) {
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Server disconnected\n");
            return EXIT_FAILURE;
        }
        buffer[bytes] = '\0';
        printf("[SERVER] %s\n", buffer);

        return on_message(context, buffer);
    }

    return EXIT_SUCCESS;

}

int get_tcp_ping_ms(int sock) {
    if (sock < 0) {
        return -1;
    }

#ifdef __linux__
    struct tcp_info info;
    socklen_t info_len = sizeof(info);
    if (getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, &info_len) == 0) {
        return (int)(info.tcpi_rtt / 1000U);
    }
#elif defined(__windows__)
    /* Windows ne fournit pas d'API simple pour obtenir le ping d'une socket TCP.
       Une solution serait de mesurer le temps entre l'envoi d'un message de ping personnalisé
       et la réception d'une réponse du serveur, mais cela nécessite une implémentation côté serveur. */
    return -1;
#elif defined(__APPLE__)
    struct tcp_connection_info info;
    socklen_t info_len = sizeof(info);
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONNECTION_INFO, &info, &info_len) == 0) {
        return (int)(info.tcpi_rtt / 1000U);
    }
#endif

    return -1;
}

int is_tcp_local_server(int sock) {
    if (sock < 0) {
        return 0;
    }

    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    if (getpeername(sock, (struct sockaddr*)&peer_addr, &peer_addr_len) != 0) {
        return 0;
    }

    if (peer_addr.sin_family != AF_INET) {
        return 0;
    }

    Uint32 addr = ntohl(peer_addr.sin_addr.s_addr);
    return ((addr & 0xFF000000U) == 0x7F000000U) ? 1 : 0;
}

int send_tcp(int sock, const char* payload) {
    
    char* message = malloc(strlen("CODENAMES ") + strlen(payload) + 1);
    if (!message) {
        perror("malloc");
        return -1;
    }

    sprintf(message, "CODENAMES %s", payload);
    int result = send(sock, message, strlen(message), MSG_DONTWAIT);
    free((void*)message);

    return result;
}

int close_tcp(int sock) {
    close(sock);
    return EXIT_SUCCESS;
}