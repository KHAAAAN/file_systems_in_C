#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

#define BLKSIZE 4096 //Block size of 4 megabytes

ENTRY entry[MAX];

int main(int argc, char *argv[]) 
{
	int i, m, r;
	char cwd[128];
	int pid, status; 
	char dirName[128], fileName[128], file1[128], file2[128];  
	char command[64];
	int parentID;
	int childID; 
	int mode;
	FILE *fp, *gp;
	int c;
	char buffer[4096]; //buffer of 4 megabytes, this is for holding the copied file name 
	int n, total = 0;
	DIR *dirp;
	struct dirent *ent;

	m = getinputs();    // get user inputs name=value into entry[ ]
	getcwd(cwd, 128);   // get CWD pathname

	printf("Content-type: text/html\n\n");
	printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

	printf("<H1>Echo Your Inputs</H1>");
	printf("You submitted the following name/value pairs:<p>");

	for(i=0; i <= m; i++){
		printf("%s = %s<p>", entry[i].name, entry[i].value);
	}
	printf("<p>");
  

  /*****************************************************************
   Write YOUR C code here to processs the command
         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/
   
	strcpy(command,entry[0].value);

	if (strcmp(command, "mkdir") == 0){
		strcpy(dirName, entry[1].value);
		mode = atoi(entry[2].value);
	} 
	else if (strcmp(command, "rmdir") == 0){
		strcpy(dirName, entry[1].value);
	} 
	else if (strcmp(command, "rm") == 0){
		strcpy(fileName, entry[1].value);
	} 
	else if (strcmp(command, "cat") == 0){
		strcpy(fileName, entry[1].value);
	} 
	else if (strcmp(command, "cp") == 0){
		strcpy(file1, entry[1].value);
		strcpy(file2, entry[2].value);
	} 
	else if(strcmp(command, "ls") == 0){
		strcpy(fileName, entry[1].value);
	}

	pid = fork();     // fork a child process to execute the cmd line
	if (pid){       
		parentID = getpid();
		printf("<p>parent PROC %d forks a child process %d\n",parentID, pid); 
		printf("<p>parent PROC %d waits\n", parentID);				 
		pid = wait(&status);       // parent sh waits for child to die 
		printf("<p>child PROC %d died: exit status = %d\n", pid, status);
		printf("<p>");
	} 
	else{
		 childID = getpid();	 
		 printf("<p>child PROC %d do_command: command=%s\n", childID, command);
		 
		if (strcmp(command, "mkdir") == 0) {
			r = mkdir(dirName, mode);  
			if (r == 0){
				printf("<p>directory %s was created sucessfully.\n", dirName);
			}
			else{
				printf("<p>mkdir(%s) was failed.\n", dirName);
			}
	 	} 

		else if (strcmp(command, "rmdir") == 0) {
			r = rmdir(dirName);  
			if (r == 0){
			  printf("<p>directory %s was removed sucessfully.\n", dirName);
			}
			else{
			  printf("<p>rmdir(%s) was failed.\n", dirName);
			}
		}

		else if (strcmp(command, "rm") == 0){
			r = unlink(fileName);  
			if (r == 0){
			  printf("<p>file %s was removed sucessfully.\n", fileName);
			}
			else{
			  printf("<p>rm(%s) was failed.\n", fileName);
	 		}
		}

		else if (strcmp(command, "cat") == 0){ //cat, cp see note pages #8, #9
			fp = fopen(fileName, "r");
			if(fp ==0){
				printf("<p>cat(%s) was failed.\n", fileName);
				printf("<p>");
				exit(1);
			}
			printf("<p>");
			while((c = fgetc(fp)) != EOF) {
				putchar(c);
				if (c == '\n'){
					putchar('\r');
					printf("<p>"); //newline html
				}
			}
			printf("<p>"); 
			fclose(fp);      
		}

		else if (strcmp(command, "cp") == 0) {
			fp = fopen(file1, "r"); //file to be copied
			
			if (fp == NULL){ 
				exit(1);
			}
			
			gp = fopen(file2, "w"); //file to copy to
			
			if (gp == NULL){ 
				exit(1);
			}
			
			while (n = fread(buffer, 1, BLKSIZE, fp)) //store up to 4 megabytes of data to buffer 1 byte at a time into buffer read from fp/
			{
			  fwrite(buffer, 1, n, gp);   //write up to n number of elements to be written from buffer to gp each element being 1 byte
			  total += n;
			}
			
			printf("<p>total of %d bytes were copied from %s to %s.\n", total, file1, file2);
			
			fclose(fp); 
			fclose(gp);
		}

		else if(strcmp(command, "ls") == 0){
			if( (dirp = opendir(fileName) != NULL ) ){
				while ( (ent = readdir (dirp) ) != NULL ) {
					printf("%s", ent->d_name);
					printf("<p>");
				}		
				closedir(dirp);
				printf("HERE");
			}	
			else{
				printf("ERROR");
				perror("");
				return EXIT_FAILURE;
			}	
		}
		 
		printf("<p>");
		
	}
		   
	// create a FORM webpage for user to submit again 
	printf("</title>");
	printf("</head>");
	printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
	printf("<p>------------------ DO IT AGAIN ----------------\n");

	//------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
	printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~khan/cgi-bin/mycgi\">");

	printf("Enter command : <INPUT NAME=\"command\"> <P>");
	printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
	printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
	printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
	printf("</form>");
	printf("------------------------------------------------<p>");

	printf("</body>");
	printf("</html>");

	return 0;
}

