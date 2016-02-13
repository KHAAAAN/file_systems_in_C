//**************************** ECHO CLIENT CODE **************************
// The echo client client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netdb.h>

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int sock, r;
int SERVER_IP, SERVER_PORT; 

// clinet initialization code

int client_init(char *argv[])
{
	printf("======= client init ==========\n");

	printf("1 : get server info\n");
	hp = gethostbyname(argv[1]);
	if (hp==0){
		printf("unknown host %s\n", argv[1]);
		exit(1);
	}

	SERVER_IP   = *(long *)hp->h_addr;
	SERVER_PORT = atoi(argv[2]);

	printf("2 : create a TCP socket\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0){
		printf("socket call failed\n");
		exit(2);
	}

	printf("3 : fill server_addr with server's IP and PORT#\n");
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = SERVER_IP;
	server_addr.sin_port = htons(SERVER_PORT);

	// Connect to server
	printf("4 : connecting to server ....\n");
	r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (r < 0){
		printf("connect failed\n");
		exit(1);
	}

	printf("5 : connected OK to \007\n"); 
	printf("---------------------------------------------------------\n");
	printf("hostname=%s  IP=%s  PORT=%d\n", 
		  hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
	printf("---------------------------------------------------------\n");

	printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
	int n;
	char line[MAX], ans[MAX];
	char temp[MAX], notFound[MAX];
	FILE *fp;
	char *s;
	char c;
	DIR *dir;
	struct dirent *ent;
	int ret;
	int count;
	int SIZE;
	struct stat statbuf;

	if (argc < 3){
		printf("Usage : client ServerName SeverPort\n");
		exit(1);
	}

	client_init(argv);

	printf("********  processing loop  *********\n");
	
	while (1){
		printf("input a line : ");
		bzero(line, MAX);                // zero out line[ ]
		fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

		line[strlen(line)-1] = 0;        // kill \n at end

		if(strncmp(line, "lcat", 4) == 0){ //local
			s = strtok(line, " ");
			s = strtok(NULL, " ");

			strcpy(notFound, s);
			
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

			fp = fopen(line, "r");
			if(fp == 0 ){
				printf("cat(%s) was failed\n", notFound);		
			}
			else{
				while((c = fgetc(fp)) != EOF){
					putchar(c);
					if(c == '\n'){
						putchar('\r');
					}
				}
				printf("\n");
			}	
			fclose(fp);
		}
		else if(strcmp(line, "lls") == 0){
			getcwd( line, MAX);
			
			if ( (dir = opendir(line)) != NULL ){
				while( (ent = readdir(dir) ) != NULL){
					printf("%s ", ent->d_name );
				}
				closedir(dir);
			}

			printf("\n");

		}
		else if(strncmp(line, "lls", 3) == 0){
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
					while( (ent = readdir(dir) ) != NULL ) {
						printf("%s ", ent->d_name);
					}
					closedir(dir);
				}
				else{
					printf("lls: cannot access %s: No such file or directory", notFound);
				}				
			}
		else if(strncmp(line, "lcd", 3) == 0){
			printf("LCD\n");
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
			
			if( (ret = chdir(line)) == -1 ){
				printf(temp, "cd: %s: No such file or directory", notFound);
			}
		}
		else if (strncmp(line, "lmkdir", 5) == 0){	
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
					printf("mkdir: cannot create directory %s: No such file or directory \n", notFound);
				}
		}
		else if (strncmp(line, "lrmdir", 5) == 0){	
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
					printf("rmdir: cannot remove directory %s: No such file or directory \n", notFound);
				}
		}
		else if (strncmp(line, "lrm", 2) == 0){	
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
					printf("rm: cannot remove directory %s: No such file or directory \n", notFound);
				}
		}
		else if(strcmp(line, "quit") == 0){
			printf("quitting..\n");
			exit(1);
		}
		else{ //if server command

			// Send ENTIRE line to server
			n = write(sock, line, MAX);
			printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

			// Read a line from sock and show it
			n = read(sock, ans, MAX);
			printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
			
			if(strcmp(ans, "PUT") == 0){
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				strcpy(temp, s);
				strcpy(line, temp);	
	
				if((r = stat(line, &statbuf)) != -1){	
					r = open(line, O_RDONLY);
					snprintf(line, MAX, "%s%d", "SIZE=", statbuf.st_size);
					
					n = write(sock, line, MAX);
					s = NULL;
					strcpy(line, "");
					while(n){
						n = read(r, line, MAX);
						write(sock, line, MAX);
					}

					close(r);

					n = read(sock, line, MAX);
				}
				else{
					n = write(sock, "BAD", MAX);
				}
			}

			else if(strncmp(ans, "SIZE=", 5) == 0){
				s = ans + 5;
				SIZE = atoi(s);	
				printf("TEST SIZE = %d\n", SIZE);
					
				s = strtok(line, " ");
				s = strtok(NULL, " ");
				
				strcpy(notFound, s);

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

				count = 0;

					
				fp = fopen(line, "w");
				s = NULL;
				strcpy(temp, "");
				while(count < SIZE){
					n = read(sock, temp, MAX);
					printf("temp = %s\n", temp); 
					count += n;
					//write bytes to file
					fprintf(fp, "%s", temp);
				}
				fclose(fp);
			}
			else if(strcmp(ans, "BAD") == 0){
				printf("BAD\n");
			}		
		}

		s = NULL;
		strcpy(line, "");
	}
}


