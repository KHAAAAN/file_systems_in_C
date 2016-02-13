#include "type.h"

void chmod_file(char *fileName, char *permissions){
	int ino;
	MINODE *mip;
	int dev;
	char tempString[256];
	int iPermissions;
	char *endPtr;
	u32 octalTemp;

	octalTemp = 0777;

	ino = getino(&dev, fileName);

	if(ino == 0){
		printf("Filename does not exist\n");
		return;
	}

	mip = iget(dev, ino);	
	
	strcpy(tempString, "");
	strcpy(tempString, permissions);
	
	//string containing permission number, null pointer that will be set to 
	iPermissions = strtol(tempString, &endPtr, 8);
	iPermissions &= octalTemp;	
	
	//set to 0

	mip->INODE.i_mode = mip->INODE.i_mode >> 9;
	mip->INODE.i_mode = mip->INODE.i_mode << 9;

	mip->INODE.i_mode |= iPermissions;	

	mip->INODE.i_ctime = time(0L);

	mip->dirty = 1;
	iput(mip);
	
}
