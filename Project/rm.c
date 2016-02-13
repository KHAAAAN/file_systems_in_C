#include "type.h"

int rm_file(char *pathname){
	MINODE *mip;
	MINODE *pip;
	int ino, bno, pino;
	int dev;
	int i;
	char buf[1024];
	char tempString[256];
	char parent[256], child[256];

	ino = getino(&dev, pathname); 
	mip = iget(dev, ino);

	//check ownership
	if(running->uid != 0 && running->uid != mip->INODE.i_uid){ //super user:ok
		printf("rm: non super-user, uid doesn't match\n");	
		iput(mip);
		return 0;	
	}

	//check DIR type and not BUSY and is empty
	if(S_ISREG(mip->INODE.i_mode) == 0){
		printf("rm: Not a file.\n");
		return 0;
	}	

	//check if busy > 1 means someone else used iget on it
	if(mip->refCount > 1){
		printf("rm: Busy, refCount > 1.\n");
		return 0;
	}


	//passed above checks
	//Deallocate its block and inode

	if(mip->INODE.i_links_count == 1){	
		for(i = 0; i < 12; i++){
			if(mip->INODE.i_block[i] == 0){
				continue; //skips bdealloc if it hits this.
			}
			bdealloc(mip->dev, mip->INODE.i_block[i]); //deallocates block

		}
		//deallocate inode
		idealloc(mip->dev, mip->ino);
		iput(mip); //which clears mip->refCount = 0;
	}
	
	strcpy(tempString,"");
	strcpy(tempString, pathname);

	strcpy(child,basename(tempString));
	strcpy(parent,dirname(tempString));
	
	//7 get parent DIR's ino and MINODE pointed by pip
	pino = getino(&dev, parent);
	pip = iget(mip->dev, pino);

	//8 remove child's entry from parent directory by
	rm_child(pip, child);

	//9 decrement pip's link_count by 1
	pip->INODE.i_links_count--;

	//touch pip's atime, mtime fields
	pip->INODE.i_atime = time(0L);
	pip->INODE.i_mtime = time(0L);
	pip->INODE.i_ctime = time(0L);

	//mark pip dirty;
	pip->dirty = 1;
	iput(pip);

	return 1; //success
}

