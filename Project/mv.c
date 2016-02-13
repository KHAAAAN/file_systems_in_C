#include "type.h"

void mv_file(char *src, char *dst){
	int dev, ino;
	MINODE *mip, *dip;

	ino = getino(&dev, src);

	if(ino == 0){
		printf("mv_file: file not found.\n");
		return;
	}

	mip = iget(dev, ino);
	
	//will be the same since we are not doing lvl 3
	ino = getino(&dev, dst);
	
	if(mip->dev == dev){
		link(src, dst);
		unlink(src);	
	}

	else{
		cp_file(src, dst);
		unlink(src);
	}


	iput(mip);
}
