#include "type.h"

void make_dir(char *pathname){
	char tempBuf[256];
	char parent[256], child[256];
	int pino; //parent's ino	
	MINODE *pip; //parent MINODE
	int dev;

	strcpy(tempBuf, pathname); //basename() and parentname() alter tempBuf!!!!
	strcpy(child, basename(tempBuf));
	strcpy(parent, dirname(tempBuf));

	pino =	getino(&dev, parent); 		
	pip = iget(dev, pino);
	
	//verify parent INODE is a DIR and child does NOT exist in the parent directory
	if(S_ISDIR(pip->INODE.i_mode) == 0){
		printf("Not a Directory.\n");
		iput(pip);//put back
		return;
	}

	//now check if child does NOT exist in the parent directory
	
	if(search(pip, child) != 0){
		printf("Directory %s already exists.\n");
		iput(pip); //put back
		return;
	}

	//(4)call mymkdir(pip, child);	
	mymkdir(pip, child);	

	//increment link count of parent by 1;
	pip->INODE.i_links_count++;	
	
	//(5) inc parent inode's link count by 1;
	//touch its atime and mark it Dirty
	pip->INODE.i_atime = time(0L);
	pip->dirty = 1;

	//put that parent's inode back onto disk, release minode
	iput(pip);
}

int mymkdir(MINODE *pip, char *name){
	int i;
	char buf[1024];
	//(1) pip points at the parent minode[] of "/a/b", name is a string "c"
	
	//(2) allocate an inode and a disk block for the new directory;
	
	ino = ialloc(pip->dev);
	bno = balloc(pip->dev);

	//(3) mip = iget(dev, ino) to load the inode into a minode[] 
	//in order to write contents to the INODE in memory.
	
	mip = iget(pip->dev, ino);
	
	//(4) write contents to mip->INODE to make it as a DIR.
	ip = &(mip->INODE); //point ip to address of mip's inode
	
	//set inode's fields	
	ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	ip->i_uid  = running->uid;	// Owner uid 
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes 
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = time(0L);  // set to current time	
	ip->i_ctime = time(0L);
	ip->i_mtime = time(0L);
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
	ip->i_block[0] = bno;             // new DIR has one data block  
	for(i = 1; i < 15; ++i){
		ip->i_block[i] = 0;
	}

	mip->dirty = 1; //mark minode dirty
	iput(mip); //release that inode, change things on the disk letting
	//it's blocks know a new inode is now in use.
	
	//***** create data block for new DIR containing . and .. entries ******
	//(6) Write . and .. entries into a buf[] of BLKSIZE
	
	//first take care of .
	get_block(mip->dev, bno, buf);		
	cp = (char *) buf;

	dp = (DIR *) cp;
	dp->inode = ino;
	dp->rec_len = 4 * ((8 + strlen(".") + 3) / 4);//formula for rec-len 
	dp->name_len = strlen("."); //sets to 1
	dp->file_type = EXT2_FT_DIR;
	strncpy(dp->name, ".", dp->name_len); //dont copy the '\0' in
				
	//go to the next record, take care of ..
	cp += dp->rec_len;
	
	dp = (DIR *) cp;
	dp->inode = pip->ino;//parent's ino.
	dp->rec_len = BLKSIZE - (4 *((8 + strlen(".") + 3) / 4));
	//have to include remaining space, we subtract .'s rec_len from BLK SIZE	
	dp->name_len = strlen("..");
	dp->file_type = EXT2_FT_DIR;
	strncpy(dp->name, "..", dp->name_len);	

	//now put that block back into device
	put_block(mip->dev, bno, buf); 
	//ip->i_block[0] is bno because we're putting back into the start of that block	

	//(7) Finally, enter name ENTRY into parent's directory by 
    // enter_name(pip, ino, name);			
	enter_name(pip, ino, name);
}

int enter_name(MINODE *pip, int myino, char *myname){
	int i;
	int idealLength;
	int needLength;
	int remain;
	char tempBuf[256];
	char buf[1024];

	i = 0;

	while(i < 12){
		if (pip->INODE.i_block[i] == 0){
			break;
		}
	
		//(1) get parent's data block into a buf[];	
		get_block(pip->dev, pip->INODE.i_block[i], buf);	

		/*(2) EXT2 DIR entries: Each DIR entry has rec_len and name_len. Each entry's
		 ideal length is   
						 IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ]. 
		 All DIR entries in a data block have rec_len = IDEAL_LEN, except the last
		 entry. The rec_len of the LAST entry is to the end of the block, which may
		 be larger than its IDEAL_LEN.

		  --|-4---2----2--|----|---------|--------- rlen ->------------------------|
			|ino rlen nlen NAME|.........|ino rlen nlen|NAME                       |
		  --------------------------------------------------------------------------
		
		*/

		//(3) To enter a new entry of name with n_len, the needed length is
		//need_length = 4 * [ (8 + n_len + 3)/4 ] // a multiple of 4
		needLength = 4 * ( (8 + strlen(myname) + 3)/ 4 );

		//(4) Step to the last entry in a data block
		//get_block(pip->dev, pip->INODE.i_block[i], buf);

		dp = (DIR *) buf;
		cp = buf;

		//step to LAST entry in	block:
		while(cp + dp->rec_len < buf + BLKSIZE){
			strcpy(tempBuf, "");
			strncpy(tempBuf, dp->name, dp->name_len);
			tempBuf[dp->name_len] = '\0';
			
			cp += dp->rec_len;
			dp = (DIR *)cp;	
		}

		// dp Now points at last entry in block
	
		//Let remain = LAST entry's rec_len - it's ideal Length;
		idealLength = 4 * ( (8 + dp->name_len + 3) / 4 );  // last entry's idealLength
		remain = dp->rec_len - idealLength; //new last entry's length
	
		//if we have sufficient remaining space	
		if(remain >= needLength){
			//enter the new entry as the LAST entry and trim the previous entry
			//to its IDEAL_length;
			dp->rec_len = idealLength;
			cp += dp->rec_len;		

			dp = (DIR *) cp; //set to new rec
			dp->inode = myino;
			dp->rec_len = remain;
			dp->name_len = strlen(myname);
			dp->file_type = EXT2_FT_DIR;
			strncpy(dp->name, myname, dp->name_len);
			
			strcpy(tempBuf, "");
			strncpy(tempBuf, dp->name, dp->name_len);
			tempBuf[dp->name_len] = '\0';

			//(6) write data block to disk
			put_block(pip->dev, pip->INODE.i_block[i],buf);
			return 0;
		}

		i++;
	}
	
	//at this point i is an empty i block (0)
	
	//(5) Reach here means: NO space in existing data block(s)
	//Allocate a new data block;
	bno = balloc(dev); 		

	//INC parent's size by 1024
	pip->INODE.i_size += 1024;		

	//now that i_block points to that new block that we just allocated's address
	pip->INODE.i_block[i] = bno;

	//assume DIRECT BLOCKS***************************
	get_block(pip->dev, pip->INODE.i_block[i],buf);	

	cp = (char *) buf;

	dp = (DIR *) cp;
	dp->inode = myino;
	dp->rec_len = BLKSIZE;
	dp->name_len = strlen(myname);
	dp->file_type = EXT2_FT_DIR;
	strncpy(dp->name, myname, dp->name_len);
	
	//(6) write data block to disk
	put_block(pip->dev, pip->INODE.i_block[i],buf);
	
	return 0;
}
