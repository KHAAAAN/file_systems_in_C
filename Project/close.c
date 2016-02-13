#include "type.h"

int close_file(int fd){
	OFT *oftp;
	MINODE *mip;

	if(fd < 0 || fd >= 10){
		printf("close_file: fd out of range.\n");
		return 0;
	}
	
	if(running->fd[fd] == NULL){
		printf("close_file: fd not used.\n");
		return 0;	
	}

	oftp = running->fd[fd];
	running->fd[fd] = 0;

	oftp->refCount--;
	
	//last user of this OFT entry ==> dispose of the Minode[]
	if(oftp->refCount > 0){ //not last user
		return 0;
	}	
	
	mip = oftp->inodeptr; //which is the file
	iput(mip);
	
	//free malloc'd stuff
	free(oftp);		
}
