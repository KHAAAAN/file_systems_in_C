#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

char buffer[2048], cwd[2048]; //we'll use 2 megabytes each to stay on the safe side
char cmd[128];
char PATH[1024], HOME[1024];
char PATHBUFFER[1024]; //need this because of what strtok does.
FILE *fp;
char *myargv[128], **myenv;
int pid, ret, status; 

void findEnvar(char *envar, char *path, char **env){ // envar must be HOME= or PATH=
	char **envp = env;
	int len = strlen(envar);

	while(*envp){ //the last string pointed to in char *argv[] and char *env is 0
		strcpy(buffer, *envp);

		if(strlen(buffer) > len){	
			buffer[5] = 0; //null terminate so buffer becomes "HOME=" or "PATH="	
						
			if(strcmp(buffer, envar) == 0){
				strcpy(path, *envp + len);
				break;
			}
		}

		envp++;
	}
}

void initialize(char **env){
	findEnvar("PATH=", PATH,  env);
	findEnvar("HOME=", HOME, env);
}


void searchPATH(){
	char *s;
	//first check if the cmd is present in PATH	
	strcpy(PATHBUFFER, PATH);
	s = strtok(PATHBUFFER, ":");
			
	do{
		//since each path simply provides a FILE struct, we will point to it until we find it
		strcpy(buffer, s);
		strcat(buffer, "/");
		strcat(buffer, cmd);	
		if((fp = fopen(buffer, "r")) != 0){	
			break;
		}	
	}while(s = strtok(0, ":"));			
}

int do_command(char *cmdLine){
	int i;
	char *s;
	char temp[128];
	strcpy(temp, cmdLine);

	s = strtok(temp, " ");
	i = 0;

	//first find the cmdLine's cmd
	strcpy(cmd, s);
	//next get cmd's arguments.
	do //copy cmdLine into tokens into myargv delimitted by space
	{
		myargv[i] = (char *) malloc(strlen(s) + 1);
		strcpy(myargv[i++], s);
	}while(s = strtok(0, " "));
	
	myargv[i] = 0; //null terminates for execve()
	searchPATH();
	i = 0;
	//first search if exists
	while(myargv[i] != 0){
		if(strcmp(">", myargv[i]) == 0){
			close(1); //close the fd[1] for this child process		
			open(myargv[i + 1], O_CREAT|O_WRONLY, 0644);	
			myargv[i] = 0; //set the new null destination for execve
			break;
		}
		else if(strcmp(">>", myargv[i]) == 0){
			close(1);
			open(myargv[i + 1], O_APPEND|O_WRONLY, 0644);
			myargv[i] = 0; //set the new null destination for execve
			break;
		}
		else if(strcmp("<", myargv[i]) == 0){
			close(0); // system call to close file descriptor 0
			open(myargv[i + 1], O_RDONLY);
			myargv[i] = 0; //set the new null destination for execve
			break;
		}
		i++;	
	}
	execve(buffer, myargv, myenv);
}

int scan(char *cmdLine, char *head, char *tail){
	//divide cmdLine into head and tail by rightmost | symbol
	int i;
	i = strlen(cmdLine) - 1;
	while(i >= 0){
		if(cmdLine[i] == '|'){
			strcpy(head, cmdLine);
			head[i] = 0;
			strcpy(tail, cmdLine + (i + 1));
			break;
		}	
		i--;	
	}		
	
	if(strcmp(tail, "") == 0){
		return 0;
	}
	return 1;
}

int do_pipe(char *cmdLine, int *pd){
	int hasPipe;
	int lpd[2];
	char head[128], tail[128]; 
	
	strcpy(head, "");
	strcpy(tail, "");
	hasPipe = 0;

	if(pd){ //if a pipe has passed in as WRITER on pipe pd:
		close(pd[0]); //close READ
		dup2(pd[1], 1); //replace fd 1 with pd[1]'s value (pipe's WRITE)
		close(pd[1]); //close WRITE
	}

	//divide cmdLine into head, tail by rightmost pipe symbol
	hasPipe = scan(cmdLine, head, tail);
	if (hasPipe){
		//create a pipe lpd;
		pipe(lpd);

		pid = fork();
		if(pid){ //parent
			//as reader on lpd:
			close(lpd[1]);
			dup2(lpd[0], 0);//replace fd 0
			close(lpd[0]);
			do_command(tail);
		}
		else{ //child
			do_pipe(head, lpd);
		}
	}
	else{
		do_command(cmdLine);
	}

}

int main(int argc, char *argv[], char *env[]){

	char *s;
	int i;
	char cmdLine[1024];

	getcwd(cwd, sizeof(cwd)); //get the current working directory
	
	myenv = env;	
	//initialize PATH and HOME environment variable
	initialize(env);
	while(1){	
		printf("mysh@LAB3:%s$ ", cwd);
		fgets(cmdLine, 2048, stdin);
		cmdLine[strlen(cmdLine) - 1] = 0; //get rid of /r
		strcpy(buffer, cmdLine);	
		s = strtok(buffer, " ");

		i = 0;
		do //copy cmdLine into tokens into myargv delimitted by space
		{
			myargv[i] = (char *) malloc(strlen(s) + 1);
			strcpy(myargv[i++], s);
		}while(s = strtok(0, " "));
		
		myargv[i] = 0; //null terminates for execve()
		
		strcpy(cmd, myargv[0]);	

		if(strcmp(cmd, "cd") == 0){
			
			if(myargv[1] != 0){ //myargv[0] = cd .. myargv[1] = dirname(/home/jay for example)
				if((ret = chdir(myargv[1])) < 0){
					printf("mysh: cd: %s: No such file or directory\n", myargv[1]);
				}
				else{ //the ret is 0		
					if(myargv[1][0] == '/'){ //absolute
						strcpy(cwd, myargv[1]);	
					}
					else{ //relative
						strcat(cwd, "/");
						strcat(cwd, myargv[1]);
					}						
				}
			}
			
			else{ //change to home directory. due to empty arg	
				ret = chdir(HOME);
				strcpy(cwd, HOME);	
			}
		}
		
		else if(strcmp(cmd, "exit") == 0){	
			exit(1);
		}

		else{
			searchPATH(); //searches if path exists, opens fp if it does, changes buffer

			if(fp){	
				fclose(fp);	
				
				//before anything here, check for file redirection
				pid = fork();
				if(pid){		
					//if pid not 0, it is parent
					pid = wait(&status);
				}
				else{ //if pid is 0 then it is the child process	
					do_pipe(cmdLine, 0);	
				}
			}
			else{ //if any other command
				pid = fork();
				if(pid){ //PARENT
					printf("PARENT %d WAITS FOR CHILD %d to DIE\n", getpid(), pid);
					pid = wait(&status);
					printf("DEAD CHILD=%d, HOW=%04x\n", pid, status);
				}
				else{ //child
					printf("child %d dies by exit(VALUE)\n", getpid());
					exit(100);
				}
				pid = wait(&status);	
			}
				
		}
	}
	
	return 0;
}
