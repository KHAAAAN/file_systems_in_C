#include "type.h"

void symlink(char *oldFileName, char *newFileName){
	char tempString[256];
	int ino, nno;
	int dev;
	MINODE *mip, *nip;
	char /*oldParent[256],*/ oldChild[256];
	char newParent[256], newChild[256];

	//(1)verify oldFileName exists	
	ino = getino(&dev, oldFileName);
	mip = iget(dev, ino);
	
	if(S_ISREG(mip->INODE.i_mode) == 0 && S_ISREG(mip->INODE.i_mode) == 0){
		printf("Not a File or directory.\n");
		return;
	}
	
	iput(mip); //put mip back

	//(2)creat a FILE /x/y/z (newFileName)
	creat_file(newFileName);
	
	//(3)change /x/y/z's type to LNK (0120000)
	nno = getino(&dev, newFileName);
	nip = iget(dev, nno);
	nip->INODE.i_mode = 0120777;
	nip->INODE.i_size = strlen(oldFileName); //we have to change this to accomodate below.

	//&& of i_block is type int *
	memcpy(nip->INODE.i_block, oldFileName, strlen(oldFileName));

	
	iput(nip);
	//NOTE: Absolute path works as if the device is not mounted
}
