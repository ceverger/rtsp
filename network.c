#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "network.h"
//
int hostIpaddr(char *ipaddr)
{
	if(ipaddr == NULL) return -1;

	bzero(ipaddr, IPV4_ADDRESS_LEN);
	
	int ret, number = 0;
	struct ifaddrs *ifa;
	
	ret = getifaddrs(&ifa);
	if(ret < 0)
	{
		perror("getifaddr()");
		return -1;
	}

	while(ifa != NULL)
	{
		if(strcmp(ifa->ifa_name, "enp0s3") == 0 || strcmp(ifa->ifa_name, "eth0") == 0)
		{
			struct sockaddr_in *addr = (struct sockaddr_in *) ifa->ifa_addr;
			if(inet_ntop(addr->sin_family, &addr->sin_addr, ipaddr, IPV4_ADDRESS_LEN) < 0)
			{
				perror("inet_ntop");
				return -1;
			}

			printf("number = %d, %s\n", number, ipaddr);
			if(*ipaddr != '\0') break;
			number++;
		}

		ifa = ifa->ifa_next;
	}

	return 0;
}

int socketPort(int sockfd)
{
	int port = -1;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	
 	bzero(&addr, sizeof(addr));

	if(getsockname(sockfd, (struct sockaddr *) &addr, &len) < 0)
	{
		perror("getsockname()");
		return -1;
	}

	port = ntohs(addr.sin_port);

	return port;
}

int socketIpaddr(int sockfd, char *ipaddr)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	bzero(&addr, sizeof(addr));

	if(getsockname(sockfd, (struct sockaddr *) &addr, &len) < 0)
	{
		perror("getsockname()");
		return -1;
	}

	if(inet_ntop(addr.sin_family, &addr.sin_addr, ipaddr, IPV4_ADDRESS_LEN) < 0)
	{
		perror("inet_ntop");
		return -1;
	}

	return 0;
}

int socketInfo(int sockfd, char *ipaddr, int *port)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	bzero(&addr, sizeof(addr));

	if(getsockname(sockfd, (struct sockaddr *) &addr, &len) < 0)
	{
		perror("getsockname()");
		return -1;
	}

	if(inet_ntop(addr.sin_family, &addr.sin_addr, ipaddr, IPV4_ADDRESS_LEN) < 0)
	{
		perror("inet_ntop");
		return -1;
	}
		
	*port = ntohs(addr.sin_port);
}

int tcpSocketOpen(bool nonblock)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	if(nonblock == false)
	{
		if(fcntl(sockfd, F_SETFL, O_NONBLOCK))
		{
		  perror("fcntl()");
		  return -1;        
		}			
	}
	
	return sockfd;
}

int tcpSocketReceive(int sockfd, void *buf, int bufsize)
{
	int ret = read(sockfd, buf, bufsize);

   if(ret < 0) 
	{
		perror("read()");
		return -1;
	}

   return ret;
}

int tcpSocketSend(int sockfd, void *buf, int bufsize)
{
	int ret = write(sockfd, buf, bufsize);

   if(ret < 0) 
	{
		perror("write()");
		return -1;
	}

   return ret;
}

int tcpSocketServer(const char *ipaddr, int port, int maxconn, bool nonblock)
{
	int sockfd;
	socklen_t len;
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(ipaddr == NULL)
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if(inet_pton(addr.sin_family, ipaddr, &addr.sin_addr) < 0)
		{
			perror("inet_pton()");
			return 1;
		}
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	if(bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		perror("bind()");
		return -1;
	}

	if(listen(sockfd, maxconn) < 0)
	{
		perror("listen()");
		return -1;
	}

	if(nonblock == false)
	{
		if(fcntl(sockfd, F_SETFL, O_NONBLOCK))
		{
		  perror("fcntl()");
		  return -1;        
		}			
	}

	return sockfd;
}

int tcpSocketConnection(int sockfd, char *ipaddr, int *port)
{
	int connfd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	bzero(&addr, sizeof(addr));

	connfd = accept(sockfd, (struct sockaddr *) &addr, &len);
	if(connfd < 0)
	{
		perror("accept()");
		return -1;
	}

	if(ipaddr != NULL)
	{
		if(inet_ntop(addr.sin_family, &addr.sin_addr, ipaddr, IPV4_ADDRESS_LEN) < 0)
		{
			perror("inet_ntop");
			return -1;
		}
	}

	if(port != NULL)
	{		
		*port = ntohs(addr.sin_port);
	}

	return connfd;
}

int tcpSocketConnectToHost(int sockfd, const char *ipaddr, int port)
{
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));		
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(ipaddr == NULL)
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if(inet_pton(addr.sin_family, ipaddr, &addr.sin_addr) < 0)
		{
			perror("inet_pton()");
			return 1;
		}
	}

	if(connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		perror("connect()");
		return 1;
	}

	return 0;	
}


int udpSocketOpen(bool nonblock)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	if(nonblock == false)
	{
		if(fcntl(sockfd, F_SETFL, O_NONBLOCK))
		{
		  perror("fcntl()");
		  return -1;        
		}			
	}
	
	return sockfd;
}

int udpSocketReceive(int sockfd, void *buf, int bufsize, char *ipaddr, int *port)
{
	int ret;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	bzero(&addr, len);

	ret = recvfrom(sockfd, buf, bufsize, 0, (struct sockaddr *) &addr, &len);

	if(ret < 0)
	{
		perror("recvfrom()");
		return -1;		
	}	

	if(ipaddr != NULL)
	{
		if(inet_ntop(addr.sin_family, &addr.sin_addr, ipaddr, IPV4_ADDRESS_LEN) < 0)
		{
			perror("inet_ntop");
			return -1;
		}
	}

	if(port != NULL)
	{		
		*port = ntohs(addr.sin_port);
	}

	return ret;
}

int udpSocketSend(int sockfd, void *buf, int bufsize, const char *ipaddr, int port)
{
	int ret;
	struct sockaddr_in addr;
	
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	if(inet_pton(AF_INET, ipaddr, &addr.sin_addr) <= 0)
	{
		perror("inet_pton");
		return -1;
	}

	ret = sendto(sockfd, buf, bufsize, 0, (struct sockaddr*) &addr, sizeof(addr));
	if(ret < 0)
	{
		perror("sendto()");
		return -1;		
	}	

	return ret;	
}


int udpSocketServer(const char *ipaddr, int port, bool nonblock)
{
	int sockfd;
	socklen_t len;
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(ipaddr == NULL)
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if(inet_pton(addr.sin_family, ipaddr, &addr.sin_addr) < 0)
		{
			perror("inet_pton()");
			return 1;
		}
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	if(bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		perror("bind()");
		return -1;
	}

	if(nonblock == false)
	{
		if(fcntl(sockfd, F_SETFL, O_NONBLOCK))
		{
		  perror("fcntl()");
		  return -1;        
		}			
	}

	return sockfd;	
}
