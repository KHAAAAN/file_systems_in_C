#include "type.h"

void menu()
{
	//display cmds that can be done
	#ifdef DEBUG
	printf("\nLevel 1 commands************\n");
	#endif
	
	printf("List of possible commands:\n");
	printf("menu, mkdir, cd, pwd, ls\n");
	printf("rmdir, creat, link, unlink, symlink\n");
	printf("rm, chmod, chown, stat, touch\n");
	
	#ifdef DEBUG
	printf("\nLevel 2 commands************\n");
	#endif
	
	printf("open, close, pfd, lseek, access\n");
	printf("read, write, cat, cp, mv\n");
	
	#ifdef DEBUG
	printf("\nLevel 3 commands************\n");
	#endif
	
	printf("mount, umount, cs, fork, ps\n");
	printf("sync, quit\n\n");
}
