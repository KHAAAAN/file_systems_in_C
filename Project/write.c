#include "type.h"

int write_file(int fd, char buf[], int bytes){
	int lbk;
	int start;
	OFT *oftp = running->fd[fd];
	int offset;
	char tempBuf[2048]; //tempBuf is what we're writing to MINODE *mip; int blk; char *cp;
	char *cq = buf;
	int remain;
	int i;
	int *pBlock;
	int count;
	int blk;

	if(fd < 0 || fd > 9){
		printf("write_file: error\n");
		return 0;
	}

	mip = oftp->inodeptr;
	
	if(oftp == NULL){
		printf("write_file: no such fd\n");
		return 0;
	}

	if(oftp->mode != 1 && oftp->mode != 2 && oftp->mode != 3){
		printf("read_file: file opened but not for reading\n");
		return 0;
	}
	
	offset = oftp->offset;

		

	while(bytes > 0){
		lbk = oftp->offset / BLKSIZE; //i_block index
		start = oftp->offset % BLKSIZE; //offset in that block		

		if(lbk < 12){
			if(mip->INODE.i_block[lbk] == 0){
				mip->INODE.i_block[lbk] = balloc(mip->dev); //must allocate a block	
				
			}

			blk = mip->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < 256 + 12){

			if(mip->INODE.i_block[12] == 0){	//is the first indirect a null address?
				mip->INODE.i_block[12] = balloc(mip->dev);	//allocate a block for that indirect	
				
				get_block(mip->dev, mip->INODE.i_block[12],tempBuf);
				pBlock = (int *) tempBuf;

				for(i = 0; i < 256; i++){  //in that block we allocated 256 addresses or 256*4 bytes which old int valuesa
					//(these are our i_block addresses) // only one level of indirection
					pBlock[i] = 0;						
				}
			}

			//we now get the start of those i_blocks
			get_block(mip->dev, mip->INODE.i_block[12],tempBuf);
			pBlock = (int *) tempBuf;
			
			if(pBlock[lbk - 12] == 0){//lbk -12, indexing so 0 255 instead of 12-267 (256 blocks)
				pBlock[lbk - 12] = balloc(mip->dev); //data block
				//we put block here too fam

			}

			blk = pBlock[lbk - 12]; //this is the i_block pointing to the address of data block;
						
		}
		else{

			//first indirection
			if(mip->INODE.i_block[13] == 0){	
				mip->INODE.i_block[13] = balloc(mip->dev);		
				
				get_block(mip->dev, mip->INODE.i_block[13],tempBuf);
				pBlock = (int *) tempBuf;

				for(i = 0; i < 256; i++){
					pBlock[i] = 0;						
				}
			}

			get_block(mip->dev, mip->INODE.i_block[13], tempBuf);
			pBlock = (int *) tempBuf; //this is the start of our second indirection
			
			//second indirection
			//lbk starts at 268, we devide by 256 because there are only 256 indirect blocks here
			if(pBlock[(lbk - 268)/256] == 0){
				pBlock[(lbk - 268)/256] = balloc(mip->dev);
				
				get_block(mip->dev, pBlock[(lbk-268)/256], tempBuf);
				pBlock = (int *) tempBuf;

				for(i = 0; i < 256; i++){
					pBlock[i] = 0;  //second level of indirection						
				}
			}	

			get_block(mip->dev, pBlock[(lbk - 268)/256], tempBuf);
			pBlock = (int *) tempBuf; //

			blk = pBlock[(lbk - 268) % 256]; //THIS IS OUR I BLOCK COZ MAILMANS ARG 
			//now finally i_block	
		}

		//we put block later. dw. relax. for seriously.
		/*for(i = 0; i < 1024; i++){
			tempBuf[i] = '\0';
		}*/

		get_block(mip->dev, blk, tempBuf);
		cp = tempBuf + start; // cp points at start in tempBuf
		remain = BLKSIZE - start; //how much more do we have to write in this block before switching
		
		/*while(remain > 0){
			*cp++ = *cq++;
			bytes--;
			remain--;
			offset++;
			oftp->offset = offset;
			
			getchar();

			if(offset > mip->INODE.i_size){
				mip->INODE.i_size++;
			}	
			if(bytes <= 0){
				oftp->offset++;
				mip->INODE.i_size++;
				*(cq + 1) = '\0';
				put_block(mip->dev, blk, tempBuf);
				break;
			}
		}*/		
		
		//remain is just what is left in the current data block
		if(bytes > remain) //if bytes left are more than what remains in the block
		{	
			memcpy(cp, cq, 1024 * sizeof(char));
			offset += remain; //offset to where remain is
			count += remain;
			bytes -= remain; //get rid of remain since we just got these bytes
			running->fd[fd]->offset = offset; //put back in our OFT structure
				
			mip->INODE.i_size += remain;
		}
		else
		{
			memcpy(cp, cq, 1024 * sizeof(char));
			cp[bytes] = (char) EOF;
			printf("%s", cq);
			offset += bytes;
			count += bytes;
			if(mip->INODE.i_size == 0){ //there is no EOF to begin with
				mip->INODE.i_size += bytes + 1;
			}
			else{
				mip->INODE.i_size += bytes;
			}
			bytes -= bytes;
			running->fd[fd]->offset = offset; //put back in our OFT structure
			

			//EOF CASE SON
			oftp->offset = offset;
			put_block(mip->dev, blk, tempBuf);
			break;
		} 

		put_block(mip->dev, blk, tempBuf);
				
	}

	mip->dirty = 1; //mark mip dirty for iput for later
	return bytes;	
}
