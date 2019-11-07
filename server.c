#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <stdbool.h>
     
int client_socket[10];
char client_name[10][15];
char client_ip[10][16];
char client_port[10][10];

void offline(int);
void hello();
void who(int);
void int2str(int, char[]);
void name(int, char[]);
void yell(int, char[]);
void tell(int, char[], char[]);


int main(int argc , char *argv[])   
{  

	if(argc != 2) return 0;

	int serv_port;
	sscanf(argv[1], "%d", &serv_port );
	
	
       	int opt = 1;   
    	int listenfd, addrlen, connfd, clienfd, n;   
   	int max_sd;
	char sendline[1024], recvline[1024], port[10];	
    	struct sockaddr_in servaddr, cliaddr[10], addr; 
 
    	fd_set rst;   
     
    	//set client socket to -1(empty)
    	for(int i=0;i<10;i++) client_socket[i] = -1;;
         
    	//create a listen socket
    	listenfd = socket(AF_INET , SOCK_STREAM , 0);
       
    	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));  
       
    	servaddr.sin_family = AF_INET;   
    	servaddr.sin_addr.s_addr = INADDR_ANY;   
    	servaddr.sin_port = htons(serv_port);   
         
    	//bind the socket  
    	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));   
        
    	listen(listenfd, 10);
     
    //	puts("Waiting for connections ...");   
         
    	while(1)   
    	{   
        	//clear socket set 
        	FD_ZERO(&rst);
	     
	        //add socket to set
        	FD_SET(listenfd, &rst);   
	        max_sd = listenfd;   
             
        	for(int i=0;i<10;i++){

            		clienfd = client_socket[i];   

            		//if not empty add socket to set  
            		if(clienfd > 0) FD_SET( clienfd , &rst);   
                 
            		if(clienfd > max_sd) max_sd = clienfd;
        	}   
     
      		//selet the fd with action
        	select( max_sd + 1 , &rst, NULL , NULL , NULL);   
             
    		//listenfd imply new connection 
        	if (FD_ISSET(listenfd, &rst)) {   
	    		addrlen = sizeof(addr);
			
            		connfd = accept(listenfd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);

            		//send new connection message
	    		hello();
			int j;
			//pick the first available space for new client socket
            		for(int i=0;i<10;i++){
                		if( client_socket[i] == -1 ){   
                    			client_socket[i] = connfd;
					strcat(client_name[i], "anonymous");
					strcat(client_ip[i], inet_ntoa(addr.sin_addr));
					j = i;
                    			break;  
               	 		}   
           	 	
			}
			
			//build welcome message
	    		memset(sendline, '\0', 1024);
	    		strcpy(sendline, "[Server] Hello, anonymous! From: ");
	    		strcat(sendline, inet_ntoa(addr.sin_addr));
	    		strcat(sendline, ":");
			int2str(ntohs(addr.sin_port), port);
			strcat(sendline, port);

            		send(connfd, sendline, strlen(sendline)+1, 0);

			strcat(client_port[j],port);



			continue;
		    
       	 	}   
             
        	//socket input available 
        	for(int i=0;i<10;i++){  	

            		clienfd = client_socket[i];   
            
           		if( FD_ISSET(clienfd , &rst) ){   
		
				memset(recvline, '\0', 1024);
				n = recv( clienfd, recvline, 1024, 0);
	
        	        	//closing
                		if (n == 0){    
                    			close(clienfd);   
					client_socket[i] = -1;	
		    			offline(i);
					continue;
               	 		}   
                     
                		//other command 
               		 	else{   
					char cmd[100];
					memset(&cmd, '\0', sizeof(char)*100);	
					for(int j=0;j<100;j++){
						if(recvline[j] == ' ') break;
						else {
							cmd[j] = recvline[j];
							recvline[j] = ' ';
						}
					
					}

					if( strcmp(cmd, "name") == 0) {
						
						int j = 0,k = 0;
						memset(cmd, '\0', sizeof(char)*100);

						while(recvline[j] != '\0' && k < 100 && j-1<1024){
							if(recvline[j++] == ' ') continue;
							else cmd[k++] = recvline[j-1];
						
						}
						name(i, cmd);
               				}

					else if( strcmp(cmd, "who") == 0){
						who(i);
					}
				     	
					else if( strcmp(cmd, "yell") == 0){

						int j=0;
						while(recvline[j] != '\0' && j+5<1024){
							recvline[j] = recvline[j+5];
							j++;
						}

						yell(i, recvline);
					}

					else if( strcmp(cmd, "tell") == 0){
						
						memset(cmd, '\0', 100);
						int j = 0;

						while(recvline[j+5] != ' ' && j < 100){
							cmd[j] = recvline[j+5];
							j++;
						}
						j++;

						int k=0;
						while(recvline[k] != '\0' && k+j+5<1024){
							recvline[k] = recvline[k+j+5];
							k++;
						}

						tell(i, cmd, recvline);
					}

					else send(clienfd, "[Server] ERROR: Error command.", 31, 0);

           	 		}	   
        		}   
   	
	       	}
	}	
        
    return 0;   
}  

void offline(int i)
{
	char sendline[100];
	memset(sendline, '\0', 100);
	strcat(sendline, "[Server] ");
	strcat(sendline, client_name[i]);
	strcat(sendline, " is offline.");

	memset(client_name[i], '\0', strlen(client_name[i]));
	memset(client_ip[i], '\0', strlen(client_ip[i]));
	memset(client_port[i], '\0', strlen(client_port[i]));

	for(int i=0;i<10;i++){
		int fd = client_socket[i];
		if(fd != -1) send(fd,sendline, strlen(sendline)+1, 0);
	}
}

void hello()
{	
	int fd;
	for(int i=0;i<10;i++){
		fd = client_socket[i];
		if(fd != -1) send(fd, "[Server] Someone is coming!", 27, 0);
	}
}

void yell(int i, char msg[])
{
	char sendline[1024];
	memset(sendline, '\0', 1024);
	strcat(sendline, "[Server] ");
	strcat(sendline, client_name[i]);
	strcat(sendline, " yell ");
	strcat(sendline, msg);

	for(int j=0;j<10;j++){
		int fd = client_socket[j];
	
		if(fd != -1) send(fd, sendline, sizeof(sendline), 0); 
	}
}

void tell(int i, char tar_name[], char msg[])
{
	int tar;
	bool err = false;
	bool exist = false;
	bool is_anony = false;
	char sendline[1024];
	memset(sendline, '\0', 1024);

	if( strcmp(client_name[i], "anonymous") == 0) {
		err = true;
		send(client_socket[i], "[Server] ERROR: You are anonymous.",35, 0);
	}

	if( strcmp(tar_name, "anonymous") == 0) {
		is_anony = true;
		err = true;
		send(client_socket[i], 
				"[Server] ERROR: The client to which you sent is anonymous.",59, 0);
	}

	for(int j=0;j<10;j++){
		if( strcmp(client_name[j], tar_name) == 0 && !is_anony){
			tar = j;
			exist = true;
			break;
		}
	}

	if(!exist) {
		err = true;
		send(client_socket[i], "[Server] ERROR: The receiver doesn't exist", 43, 0);
	}

	if(!err){
		
		send(client_socket[i], "[Server] SUCCESS: Your message has been sent.", 46, 0);

		strcat(sendline,"[Server] ");
		strcat(sendline, client_name[i]);
		strcat(sendline, " tell you ");
		strcat(sendline, msg);
		send(client_socket[tar], sendline, sizeof(sendline), 0);
	}
	
}

void name(int i, char new_name[])
{
	bool err = false;
	char send_line[100];
	char send_change[100];
	
	if( strcmp(new_name, "anonymous") == 0){
		
		err = true;
		send(client_socket[i], 
			"[Server] ERROR: Username cannot be anonymous.", 46, 0);
	}
	
	//check repeat name
	if(!err){
		for(int j=0;j<10;j++){;
			if( strcmp( new_name, client_name[j]) == 0 && i!=j){
				memset(send_line, '\0', 100);
				strcat(send_line, "[Server] ERROR: ");
				strcat(send_line, new_name);
				strcat(send_line, " has been used by others.");
				send(client_socket[i], send_line, sizeof(send_line)+1, 0);
				err = true;
			}

		}
	}

	//check format
	if(!err){
		if( strlen(new_name) < 2 || strlen(new_name) > 12) err = true;
		
		for(int j=0;j<strlen(new_name);j++){
			char c = new_name[j];
			//valid
			if( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) continue;
			else {
				err = true;
				break;
			}
		}

		if(err) send(client_socket[i], 
				"[Server] ERROR: Username can only consists of 2~12 English letters.", 67, 0);

	}

	if(!err) {
		memset(send_line, '\0', 100);
		strcat(send_line, "[Server] " );
		strcat(send_line, client_name[i]);
		strcat(send_line, " is now known as ");
		strcat(send_line, new_name);
		strcat(send_line, ".");
		
		memset(send_change, '\0', 100);
		strcat(send_change, "[Server] You're now known as ");
		strcat(send_change, new_name);
		strcat(send_change, ".");


		for(int j=0;j<10;j++){
			int fd = client_socket[i];
			if(fd == -1) continue;
			else if(i != j) send(client_socket[j], send_line, sizeof(send_line)+1, 0);
			else send(client_socket[i], send_change, sizeof(send_change)+1, 0);
		}
		//client_name[i] = new_name;
		memset(client_name[i], '\0', 15);
		strcat(client_name[i], new_name);
	}
}


void who(int i)
{
	char sendline[1024];
	for(int j=0;j<10;j++){
		int fd = client_socket[j];
		if( fd != -1){
			
			memset(sendline, '\0', 1024);
			strcat(sendline, "[Server] ");
			strcat(sendline, client_name[j]);
			strcat(sendline, " ");
			strcat(sendline, client_ip[j]);
			strcat(sendline,  ":");
			strcat(sendline, client_port[j]);
			if(i == j) {

				strcat(sendline, " ->me");
			}

			send(client_socket[i], sendline, sizeof(sendline), 0);
		}
	}
}


void int2str(int i, char port[])
{
	sprintf(port, "%d", i);
}
