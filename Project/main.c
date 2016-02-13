#include "type.h"

char *cmdList[24] = {"menu", "mkdir", "cd", "pwd", "ls", "rmdir", "creat", "link", "unlink", "symlink", "rm", "chmod",
	"chown", "stat", "touch", "open", "close", "pfd", "lseek", "read", "write", "cat", "cp", "mv"};

char pathname[64], parameter[64];

void menu_wrapper(char *pathname);
void pWorkingDir(char *pathname);
void link_file_wrapper(char *pathname);
void symlink_file_wrapper(char *pathname);
void chmod_file_wrapper(char *pathname);
void chown_file_wrapper(char *pathname);
void stat_file_wrapper(char *pathname);
void rmdir_wrapper(char *pathname);
void rm_file_wrapper(char *pathname);
void open_file_wrapper(char *pathname);
void close_file_wrapper(char *pathname);
void pfd_wrapper(char *pathname);
void lseek_wrapper(char *pathname);
void read_file_wrapper(char *pathname);
void write_file_wrapper(char *pathname);
void cp_file_wrapper(char *pathname);
void mv_file_wrapper(char *pathname);


//an array of 24 function pointers, all of which accept a char * arg.
void(*cmds[24]) (char*) = {menu_wrapper, make_dir, cd, pWorkingDir, ls, rmdir_wrapper, 
	creat_file, link_file_wrapper, unlink, symlink_file_wrapper,
   	rm_file_wrapper, chmod_file_wrapper, chown_file_wrapper, stat_file_wrapper, touch_file, open_file_wrapper, close_file_wrapper,
	pfd_wrapper, lseek_wrapper, read_file_wrapper, write_file_wrapper, cat_file, cp_file_wrapper, mv_file_wrapper};

int main(int argc, char *argv[], char *env[]){
	int cmd;
	char line[128], cname[64];
	int i;
	char diskBuf[256];

	DEBUG = 0;
	//disk = argv[1];
	
	printf("Please enter a disk to mount: ");
	fgets(diskBuf, 256, stdin);
	diskBuf[strlen(diskBuf) - 1] = '\0'; //delete new line char
	disk = diskBuf;
	printf("disk = %s\n", disk);
	init();
	
	printf("\nPossible commands to be used:\n");
	menu();
	//reccuring process to allow user to keep doing commands
	while(1) 
	{
		strcpy(pathname, "");
		strcpy(parameter, "");
		//need to get user input and split it up into cmd and input
		printf("P%d running\n\r", running->pid);
		printf("Enter the command to be executed:\n\r");
		fgets(line, 128, stdin);
		line[strlen(line)-1] = 0;  // kill the \r char at end
		if (line[0]==0) continue; //if there is no input then restart the loop

		sscanf(line, "%s %s %s", cname, pathname, parameter); 
		
		cmd = findCmd(cname, cmdList);
		
		if(cmd == -2){
			printf("exiting filesystem..\n");
			break;
		}

		if(cmd != -1) {
			(*cmds[cmd]) (pathname); //calls the function from the function ptr array
		}
	} 
	return 0;
}

void menu_wrapper(char *pathname){
	menu();
}

void pWorkingDir(char *pathname){
	pwd();
}

void link_file_wrapper(char *pathname){
	link(pathname, parameter);
}

void symlink_file_wrapper(char *pathname){
	symlink(pathname, parameter);
}

void chmod_file_wrapper(char *pathname){
	chmod_file(pathname, parameter);
}

void chown_file_wrapper(char *pathename){
	chown_file(pathname, parameter);
}

void stat_file_wrapper(char *pathname){
	struct stat someStat;
	stat_file(pathname, &someStat);
}

void rm_file_wrapper(char *pathname){
	rm_file(pathname);
}

void rmdir_wrapper(char *pathname){
	rmdir(pathname);
}

void open_file_wrapper(char *pathname){
	open_file(pathname, parameter);
}

void close_file_wrapper(char *pathname){
	close_file(atoi(pathname));
}

void pfd_wrapper(char *pathname){
	pfd();
}

void lseek_wrapper(char *pathname){
	lseek_file(atoi(pathname), atoi(parameter));
}

void read_file_wrapper(char *pathname){
	char buf[2048];
	read_file(atoi(pathname), buf, atoi(parameter));
}

void write_file_wrapper(char *pathname){
	char buf[1024];
	strcpy(buf, parameter);
	printf("buf = %s\n", parameter);
	write_file(atoi(pathname), buf, strlen(buf));
}

void cp_file_wrapper(char *pathname){
	cp_file(pathname, parameter);
}

void mv_file_wrapper(char *pathname){
	mv_file(pathname, parameter);
}
