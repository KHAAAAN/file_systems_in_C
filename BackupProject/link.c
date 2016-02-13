#include "type.h"

void link(char *oldFileName, char *newFileName){
	char tempString[256];
	int ino, npno;
	int dev;
	MINODE *mip, *npip;
	char /*oldParent[256],*/ oldChild[256];
	char newParent[256], newChild[256];

	ino = getino(&dev, oldFileName);
	mip = iget(dev, ino);

	if(S_ISREG(mip->INODE.i_mode) == 0 && S_ISLNK(mip->INODE.i_mode) == 0){
		printf("Not a file.\n");
		return;
	}

	// /a/b/c
	strcpy(tempString, "");	
	strcpy(tempString, oldFileName);
	strcpy(oldChild, basename(tempString));
	//strcpy(oldParent, dir(tempString));	

	// /x/y/z
	strcpy(tempString, "");
	strcpy(tempString, newFileName);
	strcpy(newChild, basename(tempString)); //z
	strcpy(newParent, dirname(tempString));	// /x/y
		
	//(3) check /x/y exists and is  a dir but z does not yet exist in /x/y/	
	npno = getino(&dev, newParent); //Both /a/b/c and /x/y/z must be on the SAME device.  	
	npip = iget(dev, npno);

	if(S_ISDIR(npip->INODE.i_mode) == 0){
		printf("newFileName is not a directory path.\n");
		return;
	}

	if(search(npip, newChild) != 0){
		printf("newFileName already exists.\n");
		return;
	}

	//Add an entry [ino rec_len name_len z] to the data block of /x/y/
    //This creates /x/y/z, which has the SAME ino as that of /a/b/c	
	enter_file_name(npip, ino, newChild); //from creat.c 

	npip->INODE.i_links_count++;

	iput(mip);
	iput(npip);

	return;
}
