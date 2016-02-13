#include "type.h"

char *cmdList[41] = {"menu", "mkdir", "cd", "pwd", "ls", "rmdir", "creat", "link", "unlink", "symlink", "rm", "chmod",
	"chown", "stat", "touch", "open", "close", "pfd", "lseek", "read", "write"};

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

void(*cmds[41]) (char*) = {menu_wrapper, make_dir, cd, pWorkingDir, ls, rmdir_wrapper, 
	creat_file, link_file_wrapper, unlink, symlink_file_wrapper,
   	rm_file_wrapper, chmod_file_wrapper, chown_file_wrapper, stat_file_wrapper, touch_file, open_file_wrapper, close_file_wrapper,
	pfd_wrapper, lseek_wrapper, read_file_wrapper, write_file_wrapper};

int main(int argc, char *argv[], char *env[]){
	int cmd;
	char line[128], cname[64];

	DEBUG = 0;
	//disk = argv[1];
	disk = "mydisk";
	init();
	

	printf("Possible commands to be used:\n");
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

		sscanf(line, "%s %s %s", cname, pathname, parameter); /**TODO** Make sure this is in the right order**/

		cmd = findCmd(cname, cmdList);

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
	read_file(atoi(pathname), atoi(parameter));
}

void write_file_wrapper(char *pathname){
	char buf[1024];
	strcpy(buf, parameter);
	printf("buf = %s\n", parameter);
	write_file(atoi(pathname), buf, strlen(buf));
}
