/********* super.c code ***************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>

typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;   // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

/******************* in <ext2fs/ext2_fs.h>*******************************
struct ext2_super_block {
  u32  s_inodes_count;       // total number of inodes
  u32  s_blocks_count;       // total number of blocks
  u32  s_r_blocks_count;     
  u32  s_free_blocks_count;  // current number of free blocks
  u32  s_free_inodes_count;  // current number of free inodes 
  u32  s_first_data_block;   // first data block in this group
  u32  s_log_block_size;     // 0 for 1KB block size
  u32  s_log_frag_size;
  u32  s_blocks_per_group;   // 8192 blocks per group 
  u32  s_frags_per_group;
  u32  s_inodes_per_group;    
  u32  s_mtime;
  u32  s_wtime;
  u16  s_mnt_count;          // number of times mounted 
  u16  s_max_mnt_count;      // mount limit
  u16  s_magic;              // 0xEF53
  // A FEW MORE non-essential fields
};
**********************************************************************/

char buf[BLKSIZE];
char buf2[BLKSIZE];
char *cp;
int fd;
int iblock;
int imap, bmap;  // IMAP and BMAP block number
int ninodes, nblocks, nfreeInodes, nfreeBlocks;

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  
  read(fd, buf, BLKSIZE);
}

super()
{
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK\n");

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);

  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);


  printf("s_log_block_size = %d\n", sp->s_log_block_size);
  //printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  //printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);

  printf("s_mtime = %s", ctime(&sp->s_mtime));
  printf("s_wtime = %s", ctime(&sp->s_wtime));


}

char *disk = "mydisk";

/***** SAMPLE OUTPUTs of super.c ****************
s_inodes_count                 =      184
s_blocks_count                 =     1440
s_free_inodes_count            =      174
s_free_blocks_count            =     1411
s_log_block_size               =        0
s_blocks_per_group             =     8192
s_inodes_per_group             =      184
s_mnt_count                    =        1
s_max_mnt_count                =       34
s_magic                        =     ef53
s_mtime = Mon Feb  9 07:08:22 2004
s_inode_size                   =      128
************************************************/

//2. WRITE YOUR OWN gd.c to print information of Group Descriptor 0
void groupDescriptor(){
	// read SUPER block
	get_block(fd, 2, buf);  
	gp = (GD *)buf;

	printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
	printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);	
	printf("bg_inode_table = %d\n", gp->bg_inode_table);
	printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
	printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count);
	printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count);
}

//3.
/**************** imap.c file *************************/

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;

  return 0;
}

imapFn()
{
  int  ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("bmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
}

/*********************** SAMPLE Imap ***********************************
11111111 11000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 
***********************************************************************/


//4. WRITE YOUR OWN bmap.c to print the blocks bitmap
bmapFn()
{
  int  blocks;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  blocks = sp->s_blocks_count;
  printf("blocks = %d\n", blocks);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("bmap = %d\n", bmap);

  // read inode_bitmap block
  get_block(fd, bmap, buf);

  for (i=0; i < blocks; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
}

//5. 
/********* inode.c: print information in / INODE (INODE #2) *********/

inode()
{
  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  /****************
  printf("%8d %8d %8d %8d %8d %8d\n",
	 gp->bg_block_bitmap,
	 gp->bg_inode_bitmap,
	 gp->bg_inode_table,
	 gp->bg_free_blocks_count,
	 gp->bg_free_inodes_count,
	 gp->bg_used_dirs_count);
  ****************/ 
  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  
  printf("mode=%4x ", ip->i_mode);
  printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf("size=%d\n", ip->i_size);
  printf("time=%s", ctime(&ip->i_ctime));
  printf("link=%d\n", ip->i_links_count);
  printf("i_block[0]=%d\n", ip->i_block[0]);

 /*****************************
  u16  i_mode;        // same as st_imode in stat() syscall
  u16  i_uid;                       // ownerID
  u32  i_size;                      // file size in bytes
  u32  i_atime;                     // time fields  
  u32  i_ctime;
  u32  i_mtime;
  u32  i_dtime;
  u16  i_gid;                       // groupID
  u16  i_links_count;               // link count
  u32  i_blocks;                    // IGNORE
  u32  i_flags;                     // IGNORE
  u32  i_reserved1;                 // IGNORE
  u32  i_block[15];                 // IMPORTANT, but later
 ***************************/
}

/******************************
inode_block = 10
mode=    41ed  
uid=0  
gid=0
size=1024
time=Mon Feb  9 07:32:16 2014
link=3
i_block[0]=33
******************************/

/*6. WRITE YOUR dir.c to print ALL files under the / directory:

              HOW TO STEP THROUGH dir_entries:

Assume the root directory / contains entries 
           this  is  aVeryLongName short
Then the 0th data block (i_block[0] in its inode) of this DIR contains:
 
|2 12 1.|2 12 2..|11 12 4this|12 12 2is|13 24 13aVeryLongName|14 952 5short   |

Each record has a rec_len and a name_len field.

First, read the block into a char buf[1024].
Let DIR *dp and char *cp BOTH point at buf;
Use dp-> to access the fields of the record, e.g. print its name

                    TO MOVE TO THE NEXT entry:
        cp += dp->rec_len;       // advance cp by rec_len BYTEs
        dp = (shut-up)cp;        // pull dp along to the next record

This way, you can step through ALL the record entries of a DIR file.*/

//**********NOTE*************** call inode() before this.
void dirFn(){
	int i;
	
	dp = (DIR *) buf;
	cp = (char *) dp;

	while(dp != NULL){
		printf("|inode = %d rec_len = %d name_len = %d", dp->inode, dp->rec_len, dp->name_len);	
		for(i = 0; i < dp->name_len; ++i){
			putchar(dp->name[i]);
		}
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}	
	putchar('|');
}

//7. /******* ialloc.c: allocate a free INODE, return its inode number ******/

int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
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

int decFreeInodes(int dev)
{

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

int ialloc(int dev)
{
  int  i;

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

main(int argc, char *argv[ ])
{
  int i, j, ino;
  int InodesBeginTable; //(2)
  char *s;
  char someString[128], name[128][128];
  char tempName[128];
  int blockNum, inodeNum;
  int nameCount;
  int n;
  int *pBlock, *pBlock2;
  char temp[BLKSIZE];

  if (argc > 1)
    disk = argv[1];

  fd = open(disk, O_RDWR);
  if (fd < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

//(1) Do de super block
	super();  

	//(2) 
	groupDescriptor();
	InodesBeginTable = gp->bg_inode_table; //Inode block starts here
	//(3)
	get_block(fd, InodesBeginTable, buf);
	ip = (INODE *) buf + 1;	

	//(4)
	strcpy(someString, argv[2]);
	s = strtok(someString, "/");
	nameCount = 0;
	strcpy(name[nameCount++], s);

	//(5)
	while(s = strtok(0, "/")){
		strcpy(name[nameCount++], s);
	}

	n = nameCount;	
	nameCount = 0;

	//our loop begins here (7)

	while(nameCount < n){
		get_block(fd, ip->i_block[0], buf);
		dp = (DIR *) buf;
		cp = (char *) dp;

		do{
			strcpy(tempName, ""); //reset string
			for(i = 0; i < dp->name_len; ++i){
				tempName[i] = dp->name[i];	
			}	
			tempName[i] = '\0';
			i = 0;
			printf("|inode = %d rec_len = %d name_len = %d name = %s", dp->inode, dp->rec_len, dp->name_len, tempName);		

			if(strcmp(name[nameCount], tempName) == 0){
				ino = dp->inode;
				break;	
			}
			
			cp += dp->rec_len;
			dp = (DIR *)cp;

		}while(cp < buf + BLKSIZE); //trust de wang but ask chen wei pls	

		putchar('|');
		printf("\nour tempName is %s and our name is %s.\n", tempName, name[nameCount]);	
		if(strcmp(name[nameCount++], tempName) != 0){
			printf("Not found\n");
			return 0;
		}
		
		//(8)
		else if(ino == 0){
			//can't find name[i], BOMB OUT!
			printf("BOMB OUT\n");
			return 0;
		}
	
		//(6)
		blockNum = ((ino - 1) / 8) + InodesBeginTable; //where our next i node is found
		inodeNum = (ino - 1) % 8; //our next inode's address in the block

		get_block(fd, blockNum,buf);

		//our new inode which contains "cs360"
		ip = (INODE *) buf + inodeNum;
		//we can easily determine if it is a DIR or not, if it's not a DIR, there can't be anything like /cs360/is ...., so give up.
		
		if(S_ISDIR(ip->i_mode) == 1 && name[nameCount] == NULL){
			printf("Give up\n");
			return 0;
		}
	} 
	//end of loop (8)
	printf("\n%s\n", name[nameCount]);

	//(9) print the disk blocks (i_blocks) of the found i_node
	printf("****Direct Blocks Start****\n");
	for(i = 0; i < 13; ++i){
		if(ip->i_block[i] == 0){
			printf("Test****Direct Blocks End***\n");	
			return 0;
		}
		printf("%d ", ip->i_block[i]);
	}	
	printf("****Direct Blocks End***\n");	
	getchar();
	printf("\n\n");	

	if(ip->i_block[12] == 0){
		return 0;
	}

	get_block(fd, ip->i_block[12], temp);

	i = 0;
	pBlock = (int *) temp;	
	
	printf("****Indirect Blocks Start****\n");
	
	while(i < 256){
		if(pBlock[i] == 0){
			printf("****Indirect Blocks End****\n");
			return 0;
		}
		printf("%d ", pBlock[i++]);
	}

	printf("****Indirect Blocks End****\n");
	getchar();
	printf("\n\n");

	if(i < 256 || ip->i_block[13] == 0){
		return 0;
	}	

	i = 0;
	j = 0;
	
	get_block(fd, ip->i_block[13], temp);
	getchar();

	pBlock = (int *) temp;
	
	printf("****Double Indirect Blocks Start****\n");

	while(i < 256){
		if(pBlock[i] == 0){
			printf("****Double Indirect Blocks End****\n");
			return 0;
		}
		printf("\n%d \n", pBlock[i]);

		get_block(fd, pBlock[i], buf2);
		pBlock2 = (int *) buf2;
		getchar();

		while(j < 256){
			if(pBlock2[j] == 0){
				printf("****Double Indirect Blocks End****\n");
				return 0;
			}
			printf("%d ", pBlock2[j++]);
		}
		j = 0;
		++i;
	}
	printf("****Double Indirect Blocks End****\n\n");
}
