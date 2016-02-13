#include "type.h"

int rmdir(char *pathname){
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
		printf("non super-user, uid doesn't match\n");	
		iput(mip);
		return 0;	
	}

	//check DIR type and not BUSY and is empty
	if(S_ISDIR(mip->INODE.i_mode) == 0){
		printf("Not a directory\n");
		return 0;
	}	

	//check if busy > 1 means someone else used iget on it
	if(mip->refCount > 1){
		printf("Busy, refCount > 1.\n");
		return 0;
	}

	//check is empty
	if(mip->INODE.i_links_count > 2){
		printf("Cannot remove non-empty directory\n");
		return 0;
	}		
	else{ //can still be non-empty
		for(i = 0; i < 12; i++){
			if(mip->INODE.i_block[i] == 0){
				break;
			}

			get_block(dev, mip->INODE.i_block[i], buf);
			cp = (char *) buf;
			dp = (DIR *) buf;
			
			while(cp < buf + BLKSIZE){
				strcpy(tempString, "");
				strncpy(tempString, dp->name, dp->name_len);
				tempString[dp->name_len] = '\0';
				
				if(strcmp(".", tempString) != 0 && strcmp("..", tempString) != 0){
					printf("Cannot remove non-empty directory\n");
					return 0;
				}	
				cp += dp->rec_len;
				dp = (DIR *) cp;	
			}		
		}
	}

	//passed above checks
	//Deallocate its block and inode
	
	for(i = 0; i < 12; i++){
		if(mip->INODE.i_block[i] == 0){
			continue; //skips bdealloc if it hits this.
		}
		bdealloc(mip->dev, mip->INODE.i_block[i]); //deallocates block

	}
	//deallocate inode
	idealloc(mip->dev, mip->ino);
	iput(mip); //which clears mip->refCount = 0;
	
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

int rm_child(MINODE *parent, char *name){
	char tempString[256];
	char tempBuf[1024];
	int removedLength;
	DIR *prevDp;
	int i;

	//search parent INODE's data block for the entry of name
	for(i = 0; i < 12; i++){
		get_block(parent->dev, parent->INODE.i_block[i], tempBuf);
		
		if(parent->INODE.i_block[i] == 0){
			printf("Not found\n");
			return 0;
		}

		dp = (DIR *) tempBuf;
		cp = (char *) dp;
		prevDp = (DIR *) dp;

		while(cp < tempBuf + BLKSIZE){

			strcpy(tempString, "");
			strncpy(tempString, dp->name, dp->name_len);	
			tempString[dp->name_len] = '\0';
		
			if(strcmp(tempString, name) == 0){ //if first entry in a data_block
				removedLength = dp->rec_len;
				
				if(cp == tempBuf && cp + dp->rec_len >= tempBuf + BLKSIZE){
					//full block case.

					//assume direct blocks
					for(i; i < 12; ++i){ if(i == 14){
							parent->INODE.i_block[i] = 0; //new last block because we pulled to left
							break;
						}
						parent->INODE.i_block[i] = parent->INODE.i_block[i + 1];
					}
					
					put_block(parent->dev, parent->INODE.i_block[i], tempBuf);	
					parent->dirty = 1;
					return 1;		
				}			
				if(cp + dp->rec_len >= tempBuf + BLKSIZE){
					prevDp->rec_len += removedLength;	
					
					put_block(parent->dev, parent->INODE.i_block[i], tempBuf);	
					parent->dirty = 1;
					return 1;		
				}
				else if(cp + dp->rec_len < tempBuf + BLKSIZE){ //not first entry
					
					 //if it's middle or first
					while(cp < tempBuf + BLKSIZE){ //case where it's in the front or middle
						prevDp = (DIR *) dp;
							
						cp += dp->rec_len;
						dp = (DIR *) cp;

						prevDp->inode = dp->inode;
						prevDp->name_len = dp->name_len; 	
						prevDp->file_type = EXT2_FT_DIR;
						strncpy(prevDp->name, dp->name, dp->name_len); //dont copy the '\0' in	
						if(cp + dp->rec_len >= tempBuf + BLKSIZE){
							prevDp->rec_len = removedLength + dp->rec_len;
						}
						else{
							prevDp->rec_len = dp->rec_len;  
						}
						
						if(cp + dp->rec_len >= tempBuf + BLKSIZE){
							break;
						}
					}
					
					put_block(parent->dev, parent->INODE.i_block[i], tempBuf);	
					parent->dirty = 1;
					return 1;		
				}
				
			}
			
			prevDp = (DIR *) dp; //for last case in a populated block
			cp += dp->rec_len;
			dp = (DIR *) cp;
		}	
	}

	return 0;	
}
