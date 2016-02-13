#include "type.h"

void touch_file(char *fileName){
	char buf[1024];
	MINODE *mip;
	int ino, pno;
	int dev;
	char tempString[256];

	ino = getino(&dev, fileName);
	if(ino == 0){
		//file doesnt exist ye	
		//check if parent exists
		strcpy(tempString, dirname(fileName));
		pno = getino(&dev, tempString);
		if(pno == 0){
			printf("Path does not exist\n");
			return;
		}

		//path exists
		creat_file(fileName);
		ino = getino(&dev, fileName);
	}

	mip = iget(dev, ino);
	
	mip->INODE.i_ctime = time(0L); //inode getting changed
	mip->INODE.i_mtime = time(0L);

	mip->dirty = 1; //dirty tells it we need to change it and put on disk after decrementing refCount
	iput(mip);

	return;

}
