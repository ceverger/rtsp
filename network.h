#include <stdbool.h>

#ifndef NETWORK_H
#define NETWORK_H

#define IPV4_ADDRESS_LEN 16
//
int hostIpaddr(char *ipaddr);

int socketPort(int sockfd);
int socketIpaddr(int sockfd, char *ipaddr);
int socketInfo(int sockfd, char *ipaddr, int *port);

int tcpSocketOpen(bool nonblock);
int tcpSocketReceive(int sockfd, void *buf, int bufsize);
int tcpSocketSend(int sockfd, void *buf, int bufsize);
int tcpSocketServer(const char *ipaddr, int port, int maxconn, bool nonblock);
int tcpSocketConnection(int sockfd, char *ipaddr, int *port);
int tcpSocketConnectToHost(int sockfd, const char *ipaddr, int port);

int udpSocketOpen(bool nonblock);
int udpSocketReceive(int sockfd, void *buf, int bufsize, char *ipaddr, int *port);
int udpSocketSend(int sockfd, void *buf, int bufsize, const char *ipaddr, int port);
int udpSocketServer(const char *ipaddr, int port, bool nonblock);
					 
#endif // NETWORK_H
