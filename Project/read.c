#include "type.h"

int read_file(int fd, char readBuf[], int bytes){
	int lbk, start, offset, file_size;
	int remain;
	int count = 0;
	char buf[1024];
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
			
		lbk = offset / BLKSIZE; //i_block
		start = offset % BLKSIZE; //start in the data block
		
		if(lbk < 12){
			blk = running->fd[fd]->inodeptr->INODE.i_block[lbk];
			get_block(running->fd[fd]->inodeptr->dev, blk, buf);
		}
		else if (lbk >= 12 && lbk < 256 + 12){

			
			indBlk = running->fd[fd]->inodeptr->INODE.i_block[12]; //first (only) indirect block (i_block[12])
			
			get_block(running->fd[fd]->inodeptr->dev, indBlk, buf);
			indOff = (lbk - 12); //indoff needs to be i_block - 12 because we take into account previous 12	

			pBlock = (int*) buf; //we're not at data block yet so needs to be int * to dereference and get 4 bytes
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);	
			//after indexing with that indoff, we dereference that address to get our DataBlock	
		}
		else{
			//first indirect block
			indBlk = running->fd[fd]->inodeptr->INODE.i_block[13];
		
			//get dat block son	
			get_block(running->fd[fd]->inodeptr->dev, indBlk, buf);
			indOff = (lbk - 268) / 256; //index has to start at 268 because i_block start at 268
			//we devide by 256 to know where in that 268 - (268 + 256) first level indirection we'are at
			//i.e if our actuali _block is at 0 268 - 268 / 256 = 0 meaning we have to indirect
			//from the 0th first level indirect block first.
			
			//*********MAILMANS ALGORITHM SONN PROPS TO KC WANG NA WHAT I MEAN SON********

			pBlock = (int *) buf; //buf is the address of the first indirect block still
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);

			//now that second indirection level has only 256 address. so if you want the 268 block you'll get the 
			//0th address of that second indirect block
			indOff = (lbk - 268) % 256;
			pBlock = (int *) buf;	
			get_block(running->fd[fd]->inodeptr->dev, pBlock[indOff], buf);
			//after indexing with that inoff on the indirectblock, we dereference that address to get our DataBlock	
			//address

			//printf("pBlock = %d, indOff = %d\n", pBlock, indOff);
		}
	
		//buf is whatever address of the datablock you JUST got hre 6	
		cp = buf + start; //cp points to the content in your datablock + start (offset)
		remain = BLKSIZE - start;//remain is however many bytes left to read in the datablock
		strcpy(readBuf, "");
		cq = readBuf; //cq points points to empty buffer

		//remain is just what is left in the current data block
		if(bytes > remain) //if bytes left are more than what remains in the block
		{	
			memcpy(cq, cp, 1024 * sizeof(char));
			cq[remain] = '\0';
			/*printf("%s", cq);*/
			offset += remain; //offset to where remain is
			count += remain;
			bytes -= remain; //get rid of remain since we just got these bytes
			running->fd[fd]->offset = offset; //put back in our OFT structure

		}
		else //last case
		{
			//bytes is your final remainder because we've subtracted everything up to it thus far
			memcpy(cq, cp, 1024 * sizeof(char));
			cq[bytes] = '\0';
		//	printf("%s\n", cq);
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

