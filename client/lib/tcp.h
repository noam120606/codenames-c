#ifndef TCP_H
#define TCP_H

int init_tcp(const char* server_ip, int port);
int tick_tcp(int sock);
int send_tcp(int sock, const char* message);
int close_tcp(int sock);

#endif // TCP_H