#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../include/connection_manager.h"
#include "../include/game_manager.h"
#include "../include/tcp.h"

int connection_manager_init(ConnectionManager* manager, int tcp_port) {
    if (!manager) return -1;
    manager->tcp_server = tcp_server_create(tcp_port);
    if (!manager->tcp_server) return -1;
    return 0;
}

void connection_manager_destroy(ConnectionManager* manager) {
    if (!manager) return;
    tcp_server_destroy(manager->tcp_server);
    manager->tcp_server = NULL;
}

void connection_manager_tick(ConnectionManager* manager) {
    if (!manager || !manager->tcp_server) return;
    tcp_server_tick(manager->tcp_server);
}