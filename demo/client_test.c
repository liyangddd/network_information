#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#define MAX_LINE 1024

int main(int argc,char **argv)
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int client_socket;
	int port = 8080;
	//char *str = "1,beijing,18:48:10";
	char *str = "2,张悦天";
	//char *str = "3";
	char *serverIP = "127.0.0.1";
	int n;
	if(argc > 1)
	{
	  str = argv[1];
	}
	bzero(&sin , sizeof(sin));
	sin.sin_family = AF_INET;
	inet_pton(AF_INET,serverIP,(void *)&sin.sin_addr);
	sin.sin_port = htons(port);
	
	if((client_socket = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("fail to create socket");
		exit(1);
	}
	if(connect(client_socket,(struct sockaddr *)&sin,sizeof(sin)) == -1)
	{
		perror("fail to create socket");
		exit(1);
	}
	
	n = send(client_socket, str , strlen(str) + 1, 0);
	if(n == -1)
	{
		perror("fail to send");
		exit(1);
	}
	
	n = recv(client_socket ,buf , MAX_LINE, 0);
	if(n == -1)
	{
		perror("fail to recv");
		exit(1);
	}
	printf("the length of str = %s\n" , buf);
	if(close(client_socket) == -1)
	{
		perror("fail to close");
		exit(1);
	}
	return 0;
}
