#include "type.h"

void cd(char *pathname){
	int dev;
	MINODE *mip;

	if(strcmp(pathname, "") == 0){
		iput(running->cwd); //lose the reference on your running->cwd
		running->cwd = iget(fd, 2);	//get a new reference onto root
		return;
	}
	
	//first get inode	
	ino = getino(&dev, pathname); 
	
	if(ino){ //if not ino, you'll get Give up or error message.
		mip = iget(dev,ino);
		if(S_ISDIR(mip->INODE.i_mode) != 0){
			iput(running->cwd); //lose the reference on your current running->cwd
			running->cwd = iget(dev, ino);
			return;
		}	

		//else
		printf("Not a directory\n");
	}

}

