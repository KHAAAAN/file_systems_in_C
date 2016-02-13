// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h> //for O_*
#include <dirent.h> //ls
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

// Server initialization code:

int server_init(char *name)
{
	printf("==================== server init ======================\n");   
	// get DOT name and IP address of this host

	printf("1 : get and show server host info\n");
	hp = gethostbyname(name);
	if (hp == 0){
		printf("unknown host\n");
		exit(1);
	}
	printf("    hostname=%s  IP=%s\n",
			   hp->h_name,  inet_ntoa(*(long *)hp->h_addr));

	//  create a TCP socket by socket() syscall
	printf("2 : create a socket\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		printf("socket call failed\n");
		exit(2);
	}

	printf("3 : fill server_addr with host IP and PORT# info\n");
	// initialize the server_addr structure
	server_addr.sin_family = AF_INET;                  // for TCP/IP
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
	server_addr.sin_port = 0;   // let kernel assign port

	printf("4 : bind socket to host info\n");
	// bind syscall: bind the socket to server_addr info
	r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
	if (r < 0){
		printf("bind failed\n");
		exit(3);
	}

	printf("5 : find out Kernel assigned PORT# and show it\n");
	// find out socket port number (assigned by kernel)
	length = sizeof(name_addr);
	r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
	
	if (r < 0){
		printf("get socketname error\n");
		exit(4);
	}

	// show port number
	serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
	printf("    Port=%d\n", serverPort);

	// listen at port with a max. queue of 5 (waiting clients) 
	printf("5 : server is listening ....\n");
	listen(sock, 5);
	printf("===================== init done =======================\n");
}


main(int argc, char *argv[])
{
	char *hostname;
	char line[MAX], temp[MAX], notFound[MAX];
	char *s;
	int ret;
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;
	FILE *fp;
	int count = 0;
	int SIZE;

	if (argc < 2) //this doesn't do much since you still need port number.
		hostname = "localhost";
	else
		hostname = argv[1];

	server_init(hostname); 

	// Try to accept a client request
	while(1){
		printf("server: accepting new connection ....\n"); 

		// Try to accept a client connection as descriptor newsock
		length = sizeof(client_addr);
		newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
		if (newsock < 0){
			printf("server: accept error\n");
			exit(1);
		}
		printf("server: accepted a client connection from\n");
		printf("-----------------------------------------------\n");
		printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
										ntohs(client_addr.sin_port));
		printf("-----------------------------------------------\n");

		// Processing loop
		while(1){
			n = read(newsock, line, MAX);
			if (n==0){
				printf("server: client died, server loops\n");
				close(newsock);
				break;
			}


			// show the line string
			printf("server: read  n=%d bytes; line=[%s]\n", n, line);
			
			strcpy(temp, ""); //reset temp

			if(strcmp(line, "pwd") == 0){	
				getcwd( line, MAX );
			}
			else if(strcmp(line, "ls") == 0){	
				printf("LS'd SO HARD BRO\n");
				getcwd( line, MAX);
				printf("line = %s at 141\n", line);

				if( (dir = opendir(line)) != NULL ){
					ent = readdir(dir);
					strcpy(line, ent->d_name);
					strcat(line, " ");
					while( (ent = readdir(dir) ) != NULL ) {
						strcat(line, ent->d_name);
						strcat(line, " ");
					}
					closedir(dir);
				}				
			}
			else if(strncmp(line, "ls", 2) == 0){
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				
				strcpy(notFound, s);

				//first check if local or not.
				if(s[0] != '/'){
					getcwd(temp, MAX);
					strcat(temp, "/");
					strcat(temp, s);

					strcpy(line, temp);
				}
				else{
					strcpy(temp, s);
					strcpy(line, temp);
				}
					
				if( (dir = opendir(line)) != NULL ){
					printf("line is %s with length %d\n", line, strlen(line));
					ent = readdir(dir);
					strcpy(line, ent->d_name);
					strcat(line, " ");
					while( (ent = readdir(dir) ) != NULL ) {
						strcat(line, ent->d_name);
						strcat(line, " ");	
					}
					closedir(dir);
				}
				else{
					sprintf(temp, "ls: cannot access %s: No such file or directory", notFound);
					strcpy(line, temp);
				}				
			}
			else if(strncmp(line, "cd", 2) == 0){
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				
				strcpy(notFound, s);
				printf("s[0] = %c\n", s[0]);
				//first check if local or not.
				if(s[0] != '/'){
					getcwd(temp, MAX);
					strcat(temp, "/");
					strcat(temp, s);

					strcpy(line, temp);
				}
				else{
					strcpy(temp, s);	
					strcpy(line, temp);
				}
			
				printf("At line 200, line = %s\n", temp); 
						
				if( (ret = chdir(temp)) == -1 ){
					sprintf(temp, "cd: %s: No such file or directory", notFound);
					strcpy(line, "");
					strcpy(line, temp);	
				}
			}
			else if (strncmp(line, "mkdir", 5) == 0){	
					s = strtok(line, " ");
					s = strtok(NULL, " ");
					
					strcpy(notFound, s);

					//first check if local or not.
					if(s[0] != '/'){
						getcwd(temp, MAX);
						strcat(temp, "/");
						strcat(temp, s);

						strcpy(line, temp);
					}
					else{
						strcpy(temp, s);
						strcpy(line, temp);
					}

					if( (ret = mkdir(line, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) == -1 ){
						sprintf(temp, "mkdir: cannot create directory %s: No such file or directory \n", notFound);
						strcpy(line, temp);	
					}
			}
			else if (strncmp(line, "rmdir", 5) == 0){	
					s = strtok(line, " ");
					s = strtok(NULL, " ");
					
					strcpy(notFound, s);

					//first check if local or not.
					if(s[0] != '/'){
						getcwd(temp, MAX);
						strcat(temp, "/");
						strcat(temp, s);

						strcpy(line, temp);
					}
					else{
						strcpy(temp, s);
						strcpy(line, temp);
					}

					if( (ret = rmdir(line) == -1 )){
						sprintf(temp, "rmdir: cannot remove directory %s: No such file or directory \n", notFound);
						strcpy(line, temp);	
					}
			}
			else if (strncmp(line, "rm", 2) == 0){	
					s = strtok(line, " ");
					s = strtok(NULL, " ");
					
					strcpy(notFound, s);

					//first check if local or not.
					if(s[0] != '/'){
						getcwd(temp, MAX);
						strcat(temp, "/");
						strcat(temp, s);

						strcpy(line, temp);
					}
					else{
						strcpy(temp, s);
						strcpy(line, temp);
					}

					if( (ret = unlink(line) == -1 )){
						sprintf(temp, "rm: cannot remove directory %s: No such file or directory \n", notFound);
						strcpy(line, temp);	
					}
			}
			else if(strncmp(line, "get", 3) == 0){
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				
				strcpy(notFound, s);

				//first check if local or not.
				if(s[0] != '/'){
					getcwd(temp, MAX);
					strcat(temp, "/");
					strcat(temp, s);

					strcpy(line, temp);
				}
				else{
					strcpy(temp, s);
					strcpy(line, temp);
				}

				stat(line, &statbuf);
				
				if(statbuf.st_size != 0){
					r = open(line, O_RDONLY);
					strcpy(temp, "SIZE=");
					strcpy(line, "");
					s = NULL;	
					snprintf(line, MAX, "%s%d", temp, statbuf.st_size);
					//send this back to client
					n = write(newsock, line, MAX);
					strcpy(line, "");
					while(n){
						n = read(r, line, MAX);
						write(newsock, line, MAX);
						printf("line = %s\n", line);
					}	

					close(r);
				}
				else{
					strcpy(line, "BAD");
				}
				
			}
			else if(strncmp(line, "put", 3) == 0){
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				
				strcpy(notFound, s);

				//first check if local or not.
				if(s[0] != '/'){
					getcwd(temp, MAX);
					strcat(temp, "/");
					strcat(temp, s);

					strcpy(line, temp);
				}
				else{
					strcpy(temp, s);
					strcpy(line, temp);
				}

				//ask client for the size of the file, line. Line is the path
				
				n = write(newsock, "PUT", MAX);
				n = read(newsock, temp, MAX);
				
				
					
				if(strcmp(temp, "BAD") != 0){
					printf("temp = %s\n", temp);
					s = temp + 5;
					SIZE = atoi(s);
					printf("SIZE = %d\n", SIZE);
					fp = fopen(line, "w");
					count = 0;
					while(count < SIZE){
						n = read(newsock, temp, MAX);
						count +=n;

						//write bytes to file
						fprintf(fp, "%s", temp);
					}

					fclose(fp);
					strcpy(line, "DONE");
				}
				else{
					printf("BADbruh\n");
				}

				
			}
			else{
				sprintf(temp, "command not found.\n");
				strcpy(line, temp);
			}

			n = write(newsock, line, MAX);
			
			printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
			printf("server: ready for next request\n");

			s = NULL;
			strcpy(line, "");
		}
	}
}

