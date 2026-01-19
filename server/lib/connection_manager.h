#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "tcp.h"

typedef struct {
    TcpServer* tcp_server;
} ConnectionManager;

int connection_manager_init(ConnectionManager* manager, int tcp_port);

#endif // CONNECTION_MANAGER_H