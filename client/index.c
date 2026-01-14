#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Connected to %s:%d\n", server_ip, port);
    printf("Type messages and press ENTER (Ctrl+C to quit)\n");

    char buffer[BUFFER_SIZE];

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        /* Message venant du serveur */
        if (FD_ISSET(sock, &readfds)) {
            int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("Server disconnected\n");
                break;
            }
            buffer[bytes] = '\0';
            printf("[SERVER] %s\n", buffer);
        }

        /* Entrée utilisateur */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(buffer, BUFFER_SIZE, stdin))
                break;

            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}
