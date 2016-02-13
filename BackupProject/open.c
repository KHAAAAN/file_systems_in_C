#include "type.h"

int open_file(char *pathname, char *mode){
	int ino;
	MINODE *mip;
	int dev;
	int i;
	int flag;

	if(atoi(mode) < 0 || atoi(mode) > 3){
		printf("open_file: invalid mode.\n");
	}

	ino = getino(&dev, pathname);
	if(ino == 0){
		printf("open_file: file does not exist.\n");
		return 0;
	}
	mip = iget(dev, ino);

	if(S_ISREG(mip->INODE.i_mode) == 0){
		printf("open_file: exists, but not a file.\n");
		iput(mip);
		return 0;
	}

	//5
	i = 0;
	flag = 0;
	while(i < 10){
		if(running->fd[i] != NULL){
			//read only if they're equal
			if(running->fd[i]->inodeptr->ino == mip->ino && running->fd[i]->mode != 0){
				printf("file is opened for writing. Opening read-only.\n");
				i = 0;
				while(running->fd[i] != NULL){
					i++;
				}
				flag = 1;
				break;
			}
		}
		i++;
	}

	//find null if no flag
	if(flag == 0){
		i = 0;
		while(running->fd[i] != NULL){
			i++;
		}
	}
	
	if(i == 10){
		printf("open_file: all file descriptors full.\n");
		return 0;
	}	

	running->fd[i] = (OFT *) malloc(sizeof(OFT));
	if(flag){
		running->fd[i]->mode = 0;
	}
	else{
		running->fd[i]->mode = atoi(mode); //0|1|2|3 for RD|WR|RW|APPEND
	}
	running->fd[i]->refCount++;
	running->fd[i]->inodeptr = mip;

	switch(running->fd[i]->mode){
		case 0:
			running->fd[i]->offset = 0;
			break;

		case 1:
			trunctuate(mip);
			running->fd[i]->offset = 0;
			break;
		case 2:
			running->fd[i]->offset = 0;
			break;

		case 3:
			running->fd[i]->offset = mip->INODE.i_size;
			break;
		default:
			printf("open_file: invalid mode.\n");
			return 0;
	}

		
	mip->INODE.i_atime = time(0L);
	
	return i; //i is the file descriptor	
}

