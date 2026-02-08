#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "../lib/tcp.h"

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

int tick_tcp(int sock) {
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

        // condition fin de client eventuelle du serveur
        // return 1; // Coupe le client
    }

    return EXIT_SUCCESS;

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