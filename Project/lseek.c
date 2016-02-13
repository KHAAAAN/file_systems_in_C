#include "type.h"

int lseek_file(int fd, int position){
	char buf[1024];

	get_block(running->cwd->dev, 2, buf);  
	gp = (GD *)buf;

	//left end
	if(position < 0){
		printf("lseek: cannot go below 0 position\n");
		return 0;
	}


	if(position > running->fd[fd]->inodeptr->INODE.i_size){			
		printf("lseek: cannot go outside file range.\n");
		return 0;	
	}

	//make sure position is not past double indirect
	running->fd[fd]->offset = position;

	return position;
}
