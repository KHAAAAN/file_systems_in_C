#include "type.h"

PROC P0, P1;
MINODE *root;
MINODE *mip;
int ino;
int fd;
char *disk;
char buf[1024];

int get_block(int fd, int blk, char buf[]){

	lseek(fd, (long)blk*BLKSIZE, 0);
	read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[]){

	lseek(fd, (long)blk*BLKSIZE, 0);
	write(fd, buf, BLKSIZE);
}

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
}

void mount_root(){
	//argv[1] should be mydisk
	fd = open(disk, O_RDWR);
	super(); //check for ext2_fs

	root = iget(dev, 2); /*get root inode */
	
	P0.cwd = iget(dev, 2);
	P1.cwd = iget(dev, 2);
	root->refCount = 3;	
}

super()
{
	get_block(fd, SUPERBLOCK, buf);
	sp = (SUPER *)buf;

	if(sp->s_magic != SUPER_MAGIC){
		printf("NOT an EXT2 FS\n");
		exit(1);
	}

	printf("EXT2 FS OK\n");
}

int main(int argc, char *argv[], char *env[]){
	disk = argv[1];
	ino = getino(dev, "/");
	mip = iget(dev, ino);

	iput(mip);
	mount_root();
	return 0;
}
