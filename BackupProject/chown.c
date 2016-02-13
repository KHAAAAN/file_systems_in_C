#include "type.h"

void chown_file(char *fileName, char *uid){
	MINODE *mip;
	int ino;
	int dev;
	char tempString[256];
	int iUid;

	ino = getino(&dev, fileName);
	if(ino == 0){
		printf("chown_file: Invalid pathname.\n");
		return;
	}

	printf("char *uid = %s\n", uid);	
	iUid = atoi(uid);
	printf("iUid = %d\n", iUid);	
	
	mip = iget(dev, ino);
	printf("mip->INODE.i_uid = %d\n", mip->INODE.i_uid);
	
	mip->INODE.i_uid = iUid;
	
	printf("mip->INODE.i_uid = %d\n", mip->INODE.i_uid);
	
	mip->INODE.i_ctime = time(0L);
	
	mip->dirty = 1;
	iput(mip);

	return;
}
