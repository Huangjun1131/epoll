#include <stdio.h>
#include "debug.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	if(3 != argc)
	{
		printf("Usage: %s <IP> <PORT>\n", argv[0]);
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 
	if(-1 == sockfd)
		errsys("socket");

	struct sockaddr_in serveraddr = {0};
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);//IPv4
	int len = sizeof serveraddr;

	if(-1 == connect(sockfd, (struct sockaddr*)&serveraddr, len))
		errsys("connect");

	char buf[100] = {0};
	while(1)
	{
		printf("mydatabase> ");fflush(stdout);
		gets(buf);
		int ret = write(sockfd, buf, sizeof buf);
		ret = read(sockfd, buf, sizeof buf);
		printf("%s\n", buf);
	}
	close(sockfd);
}

