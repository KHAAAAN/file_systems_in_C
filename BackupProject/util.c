#include "type.h"

/*************
 *get_block gets 
 the block we are searching for
 from the file descriptor
 *************/
int get_block(int fd, int blk, char buf[]){
	//0 (the third parameter) represents the origin, in this case the beginning of what i seek'd
	//the reason it is blk * BLKSIZE is because blk is the blk number and BLKSIZE is by how many times we have to move over to.
	lseek(fd, (long)blk*BLKSIZE, 0); //seeks the file descriptor from whatever block is given * BLKSIZE	
	read(fd, buf, BLKSIZE); //reads from our lseek'd position of file descriptor, up to BLKSIZE bytes into buf
}

/*************
 *put_block puts 
 the new block info
 into the file descriptor
 *************/
int put_block(int fd, int blk, char buf[]){
	lseek(fd, (long)blk*BLKSIZE, 0); //seeks the file descriptor from whatever block is given * BLKSIZE
	write(fd, buf, BLKSIZE); //writes into our file descriptor from buf up to BLKSIZE bytes.
}

/*************
 *tokenize breaks up pathname
  and puts each string into names[][] 
 *************/
void tokenize(char *pathname){
	int i;
	char *s;
	char tempBuf[256];
	
	//first reset all strings.
	for(i = 0; i < 64; ++i){
		strcpy(names[i], "");
	}


	if(strcmp(pathname, "/") == 0){ //this say
		return;
	}
	
	if(strcmp(pathname, "") == 0){
		pathname = ".";
	}

	strcpy(tempBuf, pathname);

	i = 0;
	
	s = strtok(tempBuf, "/");
	strcpy(names[i++], s);

	while(s = strtok(NULL, "/")){
		strcpy(names[i++], s);	
	}
}

/*************
 *init initializes the file system
 *************/
void init(){
	int i = 0;	

	P0.uid = 0;
	P0.cwd = 0;

	P1.uid = 1;
	P1.cwd = 0;

	for(i = 0; i < 100; ++i){
		minode[i].refCount = 0;
	}

	root = 0;	
	running = &P0;
	running->cwd = &(minode[0]);
		
	//open the disk for read write
	fd = open(disk, O_RDWR);
	
	//initialize inode_start
	groupDescriptor();
	inode_start = gp->bg_inode_table;
		
	//mount our root
	mount_root(fd);	

}

/*************
 *returns inode number
 *of given pathname
 *taking the mounted device into
 *account
 *************/
int getino(int *dev, char *pathname){
	
	int nameCount, blockNum, inodeNum;
	char tempName[256];	
	MINODE tempMip;	
	int iblockCount;
	char buf[1024];
	
	ino = 0;
	tokenize(pathname);	

	if(DEBUG){	
		fprintf(stderr, "entering int getino()\n");
	}

	if(strcmp(pathname, "/") == 0){
		*dev = root->dev;
		return 2; //ino 2 indicates root
	}

	if(pathname[0] == '/'){
		*dev = root->dev;
		
		blockNum = inode_start; //Inode block starts here
		inodeNum = 1;
	}
	else{
		*dev = running->cwd->dev;
		
		blockNum = (running->cwd->ino -1) / 8 + inode_start;
		inodeNum = (running->cwd->ino - 1) % 8;	//this is where the inode of cwd starts.
	
	}
		
	get_block(*dev, blockNum, buf); //this gets that very block, puts it into buf.
	ip = (INODE *) buf + inodeNum;	//we must start our INODE pointer at buf + 1, because inode indexing
	//starts at 1 (this is /'s INODE)

	nameCount = 0;
	//'\x00' signifies unitialized character in the 256 char * array
	while(strcmp(names[nameCount], "") != 0){
		
		for(iblockCount = 0; iblockCount < 12, ip->i_block[iblockCount] != 0; ++iblockCount){
			get_block(*dev, ip->i_block[iblockCount], buf); //make this for all direct blocks********
			
			ino = 0; //init ino

			dp = (DIR *) buf;
			cp = (char *) dp;
			
			do{
				if(dp->inode == 0){
					break;
				}

				strcpy(tempName, ""); //reset string	
				strncpy(tempName, dp->name, dp->name_len);	
				tempName[dp->name_len] = '\0';
				
				if(DEBUG){
					printf("|inode = %d rec_len = %d name_len = %d name = %s", dp->inode, dp->rec_len, dp->name_len, tempName);	
					printf("\n--names[nameCount] = %s, tempname =%s\n", names[nameCount], tempName);	
				}
				
				if(strcmp(names[nameCount], tempName) == 0){
					ino = dp->inode;
					break;
				}

				cp += dp->rec_len;
				dp = (DIR *) cp;

				//while: buf + BLKSIZE because buf = [_________](points to some 4 byte address) + 
				//BLKSIZE(this signifies end of buf)	
			} while(cp < buf + BLKSIZE); 
		}

		if(DEBUG){
			putchar('|');
			putchar('\n');	
		}

		if(strcmp(names[nameCount++], tempName) != 0){
			//printf("Not found\n");
			return 0;
		}

		else if(ino == 0){
			//can't find names[nameCount]
			//printf("BOMB OUT\n");
			return 0;
		}

		blockNum = ((ino - 1) / 8) + inode_start; //where our next inode is found
		inodeNum = (ino - 1) % 8; //our next inode's address in the block
			
		get_block(*dev, blockNum, buf);
			
		//our new inode which contains "cs360"
		ip = (INODE *) buf + inodeNum;
		//we can easily determine if it is a DIR or not, if it's not a DIR, there can't be anything like /cs360/is ...., so give up.
		//1 means its not a directory	
		/*if(S_ISDIR(ip->i_mode) == 1 && strcmp(names[nameCount], "") == 0){
			printf("Give up\n");
			return 0;
		}*/

	}
	
	if(DEBUG){
		printf("about to return ino: %d in getino\n", ino);
	}
	
	return ino;	
}

/****************************************************************************
 * search() searches the data blocks of a DIR inode (inside an Minode[])
 * for an inode's name. You may assume DIRECT data blocks only.
*****************************************************************************/
int search(MINODE *mip, char *name){
	int i;
	char tempName[256];
	char buf[1024];

	i = 0;
	while(i < 12){
		//first check if i_block is even initiated.
		if(mip->INODE.i_block[i] == 0){
			return 0;
		}

		//we get whatever i_block we are on
		get_block(mip->dev, mip->INODE.i_block[i], buf);
		
		dp = (DIR *) buf;
		cp = (char *) dp;
		
		do{
			strcpy(tempName, ""); //reset string	
			strncpy(tempName, dp->name, dp->name_len);	
			tempName[dp->name_len] = '\0';

			if(strcmp(tempName, name) == 0){
				return dp->inode;//return inode number
			}

			cp += dp->rec_len;
			dp = (DIR *) cp;

		}while(cp < buf + BLKSIZE);
		
		i++;
	} 
	//default return 0, hasn't found name
	return 0;
}

/****************************************************************************
* iget loads an inode into a slot of the minode[100] array or ups a refcount
* if already existing 
*****************************************************************************/
MINODE *iget(int dev, int ino){
	int i, iblockCopy;
	int inodeBlock, inodeNum;
	char buf[1024];

	if(DEBUG){	
		fprintf(stderr, "entering MINODE *iget()\n");
	}

	i = 0;

	//an empty name means an empty minode slot in the 100 array.
	while(minode[i].ino != 0){
		//we have ino as a local parameter, so we may load the inode into a slot in the MINODE minode[] array
		//first search the Minode[] array to see whether the needed INODE already exist

		if(minode[i].ino == ino && minode[i].dev == dev){
			minode[i].refCount += 1; //since the minode is already in here, it means we have one more reference to the minode now	
			return &(minode[i]); //return the address of minode[i]
		}
		i++;
	}

	i = 0;
	while(minode[i].ino != 0){
		i++;
	} //gets us to the first free MINODE in the array

	//we did not find it in memory so we will allocate a free MINODE	
	
	minode[i].dev = dev;
	minode[i].ino = ino;
	minode[i].refCount = 1;
	minode[i].dirty = 0;

	inodeBlock = (ino - 1) / 8 + inode_start; //remember to initialize inode_start in function init
	inodeNum = (ino - 1) % 8; //the offset of that inode within 
	
	get_block(dev, inodeBlock, buf); //this gets the inode block
	
	ip = (INODE *) buf + inodeNum; //adds the offset to the address buf is read from after the lseek
	//minode[i].INODE = *ip; //doesnt work for some reason?
	/*****set INODE fields******/
	minode[i].INODE.i_mode = ip->i_mode; 
	minode[i].INODE.i_uid = ip->i_uid;
	minode[i].INODE.i_size =  ip->i_size;
	minode[i].INODE.i_atime = ip->i_atime;
	minode[i].INODE.i_ctime = ip->i_ctime;
	minode[i].INODE.i_mtime = ip->i_mtime;
	minode[i].INODE.i_dtime = ip->i_dtime;
	minode[i].INODE.i_gid = ip->i_gid;
	minode[i].INODE.i_links_count = ip->i_links_count;

	//for(iblockCopy = 0; iblockCopy < 15, ip->i_block[iblockCopy] != 0; ++iblockCopy){
	for(iblockCopy = 0; iblockCopy < 15; ++iblockCopy){
		minode[i].INODE.i_block[iblockCopy] = ip->i_block[iblockCopy];
	}	
	
	return &(minode[i]);	
}

/****************************************************************************
*iput() releases a minode[100] item.
*make sure to scan the minode array beforehand with iget.
*****************************************************************************/
int iput(MINODE *mip){
	int inodeBlock, inodeNum;
	char buf[1024];
	int i;
	//First, dec the refCount by 1
	mip->refCount -= 1;

	
	//this doesnt need to be here but is more explicit.	
	/*if(mip->refCount > 0 || mip->dirty == 0){
		return mip->refCount;
	}*/

	//otherwise must write back INODE to disk
	
	if(mip->dirty == 1){
		inodeBlock = ( (mip->ino - 1) / 8 ) + inode_start;
		inodeNum = (mip->ino - 1)  % 8;

		//get block where inode is
		get_block(mip->dev, inodeBlock, buf);
		ip = (INODE *) buf + inodeNum;
        
		ip->i_mode = mip->INODE.i_mode;
        ip->i_uid = mip->INODE.i_uid;
        ip->i_size = mip->INODE.i_size;
        ip->i_atime = mip->INODE.i_atime;
        ip->i_ctime = mip->INODE.i_ctime;
        ip->i_mtime = mip->INODE.i_mtime;
        ip->i_dtime = mip->INODE.i_dtime;
        ip->i_gid = mip->INODE.i_gid;
        ip->i_links_count = mip->INODE.i_links_count;

        for(i = 0; i < 15; i++)
        {
            ip->i_block[i] = mip->INODE.i_block[i];
        }
		
		put_block(mip->dev, inodeBlock, buf);
	}

	return mip->refCount; //let us know how many references are left 
}

/****************************************************************************
*findmyname finds the name string of myino in the parent's data block. 
*This is the SAME as SEARCH() by myino, then copy its name string
*****************************************************************************/
int findmyname(MINODE *parent, int myino, char *myname){
	int i;
	char tempName[256];
	char buf[1024];

	i = 0;

	while(i < 12){
		//first check if i_block is even initiated.
		if(parent->INODE.i_block[i] == 0){
			return 0;
		}

		get_block(parent->dev, parent->INODE.i_block[i], buf);
		dp = (DIR *) buf;
		cp = (char *) dp;
	
		do{	
			if(dp->inode == 0){
				break;
			}

			if(myino == dp->inode){	
				strcpy(tempName, "");
				strncpy(tempName, dp->name, dp->name_len);	
				tempName[dp->name_len] = '\0';
				
				strcpy(myname, tempName);

				return 1;
			}	
			
			cp += dp->rec_len;
			dp = (DIR *) cp;

		}while(cp < buf + BLKSIZE);

		i++;
	}
	
	return 0;		
}

/****************************************************************************
*findindo: For a DIR Minode, extract the inumbers of . and ..
*Read in 0th data block. The inumbers are in the first two dir entries
*****************************************************************************/
int findino(MINODE *mip, int *myino, int *parentino){
	char buf[1024];

	if(mip->INODE.i_block[0] == 0){
		return 0;
	}

	get_block(mip->dev, mip->INODE.i_block[0], buf);	
	
	dp = (DIR *) buf;
	cp = (char *) dp;

	*myino = dp->inode; // . ino

	cp += dp->rec_len;
	dp = (DIR *) cp;
	
	*parentino = dp->inode;

	return 1;	
}

/****************************************************************************
* super() gets our super block which is block 1 in a ext2 device, checks
* the s_magic to see if it is an EXT2 file system or not.
*****************************************************************************/
void super(){
	char buf[1024];

	get_block(fd, SUPERBLOCK, buf);
	sp = (SUPER *)buf;

	if(sp->s_magic != SUPER_MAGIC){
		printf("NOT an EXT2 FS\n");
		exit(1);
	}

	printf("EXT2 FS OK\n");
}

/****************************************************************************
* groupDescriptor() gets our group descriptor block which is block 2 in an
* ext2 device.
*****************************************************************************/
void groupDescriptor(){
	char buf[1024];

	get_block(fd, 2, buf);  
	gp = (GD *)buf;
}

/*********************** SAMPLE Imap Block ***********************************
11111111 11000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 
***********************************************************************/
void imapBlock(){
  int  i;
  char buf[1024];

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  
  if(DEBUG)
  	printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  
  if(DEBUG)
  	printf("inode bitmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  if(DEBUG){  
	  for (i = 0; i < ninodes; i++){
		(test_bit(buf, i)) ?	putchar('1') : putchar('0');
		/*if (i && (i % 8)==0){
		   printf(" ");
		}*/

		if(i % 8 == 7){
			printf(" ");
		}
	  }
	  printf("\n");
  }
}

void bmapBlock(){
	char buf[1024];
	
	// read SUPER block
	get_block(fd, 1, buf);
	sp = (SUPER *)buf;

	nblocks = sp->s_blocks_count;
	printf("nblocks = %d\n", nblocks);
	
	//read Group Descriptor 0
	get_block(fd, 2, buf);
   	gp = (GD *)buf;
	
	bmap = gp->bg_block_bitmap;	
	printf("block bitmap = %d\n", bmap);

	//read block_bitmap block
	get_block(fd, bmap, buf);
}

int test_bit(char *buf, int bit) //buf[i] is -1 (for 1, represented as 1111 1110) and 0 is just 0
{
  int i, j;
  i = bit/8; j=bit%8;
  
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int incFreeInodes(int dev){
  char buf[1024];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev){
  char buf[1024];

  // inc free blocks count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);

}

int decFreeInodes(int dev)
{
  char buf[1024];
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  char buf[1024];
  // dec free blocks count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

/****************************************************************************
*allocates an inode on the device, and keeps track on imap
*****************************************************************************/
int ialloc(int dev){
	int i;
	char tempBuf[1024]; //we need this because decFreeInodes changes buf

	//read inode_bitmap block
	//sets imap, ninodes, buf
	get_block(dev, 1, tempBuf);
	
	sp = (SUPER *)tempBuf;

	ninodes = sp->s_inodes_count;

	// read Group Descriptor 0
	get_block(dev, 2, tempBuf);
	gp = (GD *)tempBuf;

	imap = gp->bg_inode_bitmap;

	// read inode_bitmap block
	get_block(dev, imap, tempBuf);

	if(DEBUG){
		for (i=0; i < ninodes; i++){
			(test_bit(tempBuf, i)) ?	putchar('1') : putchar('0');
			if (i % 8 == 7){
			   printf(" ");
			}
		}
		printf("\n");
	}

	for(i = 0; i <  ninodes; i++){
		if (test_bit(tempBuf, i) == 0){ //test for free inode in imap
			set_bit(tempBuf, i);
			decFreeInodes(dev); //decrement how many free inodes there are wherever we keep track of em

			put_block(dev, imap, tempBuf); //finally send info back to disk

			return i + 1; //returns the i number.
		}
	}

	return 0;
}

int balloc(int dev){
	int i;
	char tempBuf[1024];

	//read block_bitmap block
	//sets bmap, nblocks, buf
	
	get_block(dev, 1, tempBuf);
	sp = (SUPER *)tempBuf;

	nblocks = sp->s_blocks_count;
	
	//read Group Descriptor 0
	get_block(dev, 2, tempBuf);
   	gp = (GD *)tempBuf;
	
	bmap = gp->bg_block_bitmap;	

	//read block_bitmap block
	get_block(dev, bmap, tempBuf);

	if(DEBUG){
		for (i=0; i < nblocks; i++){
			(test_bit(tempBuf, i)) ?	putchar('1') : putchar('0');
			if (i % 8 == 7){
			   printf(" ");
			}
		}
	}
	
	for(i = 0; i < nblocks; i++){
		if (test_bit(tempBuf, i) == 0){
			set_bit(tempBuf, i);
			decFreeBlocks(dev);

			put_block(dev, bmap, tempBuf);
	
			return i + 1;
		}
		
	}

	return 0;	
}

int idealloc(int dev, int ino){
   	char tempBuf[1024];	
	get_block(dev, imap, tempBuf);
    clr_bit(tempBuf, ino);
    incFreeInodes(dev);
    put_block(dev, imap, tempBuf);

	return 0;
}

int bdealloc(int dev, int bno){
   	char tempBuf[1024];	
	
	//get bmap by getting the group descriptor block
	get_block(dev, 2, tempBuf);
   	gp = (GD *)tempBuf;
	
	bmap = gp->bg_block_bitmap;	
	
	get_block(dev, bmap, tempBuf);
    clr_bit(tempBuf, bno);
    incFreeBlocks(dev);
    put_block(dev, bmap, tempBuf);
}

void mount_root(int dev){
	//argv[1] should be mydisk
	//fd = open(disk, O_RDWR);
	super(); //check for ext2_fs

	root = iget(dev, 2); /*get root inode */
	
	P0.cwd = iget(dev, 2);
	P1.cwd = iget(dev, 2);
	root->refCount = 3;	
}

//allocate a FREE minode[i]; return pointer to it;
MINODE *mialloc(){

}

int midealloc(MINODE *mip){
	mip->refCount = 0;
}

int findCmd(char *cname, char **cmdList){
	int i;
	for(i = 0; i < 41; i++){
		if(strcmp(cname, cmdList[i]) == 0){
			return i;
		}
	}

	printf("Invalid command\n");
	return -1;
}

int pfd(){
	int i;			
	
	printf("fd\tmode\toffset\tINODE\n");
	printf("----\t----\t----\t----\n");
	
	i = 0;
	while(i < 10){
		if(running->fd[i] != NULL){
			printf("%d\t", i);
			switch(running->fd[i]->mode){
				case 0:
					printf("READ\t");
					break;

				case 1:
					printf("WRITE\t");
					break;
				case 2:
					printf("RW\t");
					break;

				case 3:
					printf("APPEND\t");
					break;
			}

			printf("%d\t", running->fd[i]->offset);
			printf("[%d, %d]\n", running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);
		}
		i++;
	}
}
