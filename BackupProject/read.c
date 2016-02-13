#include "type.h"

int read_file(int fd, int bytes){
	int lbk, start, offset, file_size;
	int remain;
	int count = 0;
	char buf[1024];
	char readBuf[4096];
	char *cq = readBuf;
	char *cp;
	int indBlk, indOff;
	int blk;
	int *pBlock;

	if(running->fd[fd] == NULL){
		printf("read_file: no such fd\n");
		return 0;
	}

	if(running->fd[fd]->mode != 0 && running->fd[fd]->mode != 2){
		printf("read_file: file opened but not for reading\n");
		return 0;
	}

	offset = running->fd[fd]->offset;
	file_size = running->fd[fd]->inodeptr->INODE.i_size;

	//optimization.
	if(bytes >= (file_size - offset) ){
		printf("read_file: bytes greater than file range.\n");
		bytes = file_size - offset - 1;  //offset takes us back to current position, from there we go to bytes.
		printf("bytes now = %d\n", bytes);
	}
	
	//avil = file_size - offset;
	while(bytes){
			
		lbk = offset / BLKSIZE;
		start = offset % BLKSIZE;
		
		if(lbk < 12){
			blk = running->fd[fd]->inodeptr->INODE.i_block[lbk];
			get_block(running->fd[fd]->inodeptr->dev, blk, buf);
		}
		else if (lbk >= 12 && lbk < 256 + 12){

			
			indBlk = running->fd[fd]->inodeptr->INODE.i_block[12];
			
			get_block(running->fd[fd]->inodeptr->dev, indBlk, buf);
			indOff = (lbk - 12);	

			pBlock = (int*) buf;
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);	
			
		}
		else{

			indBlk = running->fd[fd]->inodeptr->INODE.i_block[13];
			
			get_block(running->fd[fd]->inodeptr->dev, indBlk, buf);
			indOff = (lbk - 268) / 256;

			pBlock = (int *) buf;
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);

			indOff = (lbk - 268) % 256;
			pBlock = (int *) buf;	
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);

			//printf("pBlock = %d, indOff = %d\n", pBlock, indOff);
		}
		
		cp = buf + start; //cp points to the content in your datablock + start (offset)
		remain = BLKSIZE - start;//remain is however many bytes left to read in the datablock
		strcpy(readBuf, "");
		cq = readBuf; //cq points points to empty buffer

		//remain is just what is left in the current data block
		if(bytes > remain) //if bytes left are more than what remains in the block
		{	
			memcpy(cq, cp, 1024 * sizeof(char));
			cq[remain] = '\0';
			printf("%s", cq);
			offset += remain; //offset to where remain is
			count += remain;
			bytes -= remain; //get rid of remain since we just got these bytes
			running->fd[fd]->offset = offset; //put back in our OFT structure

		}
		else
		{
			memcpy(cq, cp, 1024 * sizeof(char));
			cq[bytes] = '\0';
			printf("%s", cq);
			offset += bytes;
			count += bytes;
			bytes -= bytes;
			running->fd[fd]->offset = offset; //put back in our OFT structure
			break;
		} 
		//printf("bytes = %d\n", bytes);		
	}
	return count; //count is the actual number of bytes read
}

