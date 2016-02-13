#include "type.h"

void unlink(char *fileName){
	MINODE *mip;
	MINODE *pip;
	int ino, pno;
	char tempString[256];
	int dev;
	char parent[256], child[256];
	
	ino = getino(&dev, fileName);
	if(ino == 0){
		return;
	}

	mip = iget(dev, ino);
	
	//only a file or symlink can be unlinked
	if(S_ISREG(mip->INODE.i_mode) == 0 && S_ISLNK(mip->INODE.i_mode) == 0){
		printf("Not a file.\n");
		return;
	}
		
	mip->INODE.i_links_count--;
	mip->dirty = 1;

	if(mip->INODE.i_links_count == 0){
		trunctuate(mip);			
	}

	strcpy(tempString, "");
	strcpy(tempString, fileName);
	strcpy(child, basename(tempString));
	strcpy(parent, dirname(tempString));

	pno = getino(&dev, parent);
	pip = iget(dev, pno);
	
	rm_child(pip, child);

	iput(pip); //lose ref
	iput(mip); //lose ref
	
	return;

}

void trunctuate(MINODE *mip){
	int i;
	
	for(i = 0; i < 15; ++i){
		if(mip->INODE.i_block[i] == 0){
			break;
		}
		bdealloc(mip->dev, mip->INODE.i_block[i]);
		mip->INODE.i_block[i] = 0;
		mip->dirty = 1;
		iput(mip);
		mip->refCount++;		
	}

	return;
}

