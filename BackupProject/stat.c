#include "type.h"

void stat_file(char *pathname, struct stat *statBuf){
	int ino;
	MINODE *mip;
	int dev; 

	ino = getino(&dev, pathname);
	mip =  iget(dev, ino);

	statBuf->st_dev = (dev_t) mip->dev;
	statBuf->st_ino = (ino_t) mip->ino;
	statBuf->st_mode = (mode_t)	mip->INODE.i_mode;
	statBuf->st_nlink = (nlink_t) mip->INODE.i_links_count;	
	statBuf->st_uid = (uid_t) mip->INODE.i_uid;
	statBuf->st_gid = (gid_t) mip->INODE.i_gid;
	statBuf->st_size = (off_t) mip->INODE.i_size;
	statBuf->st_blksize = (blksize_t) 1024;
	statBuf->st_blocks = (blkcnt_t) mip->INODE.i_blocks;
	statBuf->st_atime = (time_t) mip->INODE.i_atime;
	statBuf->st_mtime = (time_t) mip->INODE.i_mtime;
	statBuf->st_ctime = (time_t) mip->INODE.i_ctime;
 
	iput(mip);
	return;
}
