#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>

int max(int a, int b)
{
	return a > b ? a : b;
}


void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, n, stdineof = 0;
	fd_set rset;
	char sendline[1024], recvline[1024];
	
	while(1){

		
		FD_ZERO(&rset);
		FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) +1;
		
		memset(sendline, '\0', 1024);
		memset(recvline, '\0', 1024);

		select(maxfdp1+1, &rset, NULL, NULL, NULL);

		//socket readable
		if( FD_ISSET(sockfd, &rset) ){

			if( recv(sockfd, recvline, 1024, 0) == 0){
				if(stdineof == 1) return;
				//server terminate
				else return;
			}

			puts(recvline);
		
		}
		
		//input readable
		if(FD_ISSET(fileno(fp), &rset) ){
			if(fgets(sendline, 1024, fp) == NULL){
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rset);
				return;
			}

			else{
				for(int i=0;i<1024;i++){
					if(sendline[i] == '\n'){
						sendline[i] = '\0';
						break;
					}
				}

				if(strcmp(sendline, "exit") == 0){
					stdineof = 1;
					close(sockfd);
					return;
				}

				send(sockfd, sendline, strlen(sendline), 0);
			}
			
		}
	}
}

int main(int argc, char *argv[])
{

	if(argc != 3) return 0;
	int serv_port;
	sscanf(argv[2], "%d", &serv_port);

	struct sockaddr_in addr;
	int sockfd, connfd;
	char buffer[1024];

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serv_port);
	//addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
		
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	inet_pton(AF_INET, argv[1], &addr.sin_addr);

	connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));

	//Fputs("connect!\n", stdout);
	str_cli(stdin, sockfd);
	
	return 0;
}

