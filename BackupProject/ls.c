#include "type.h"

void ls(char *pathname){	
	int ino;
	int dev = running->cwd->dev;
	MINODE *mip = running->cwd;
	char dot[2];

	if(DEBUG){
		fprintf(stderr, "entering ls\n");
	}

	if(strcmp(pathname, "") == 0){
		//means absolute path
		if(pathname[0] == '/'){
			dev = root->dev;
		}
		ino = getino(&dev, pathname);
		mip = iget(dev, ino); //load inode into memory
	}
	else{
		strcpy(dot, ".");
		ino = getino(&dev, dot);
		mip = iget(dev, ino); //load inode into memory
	}
	
	//this ls will behave like ls -ial pathname aka inode hidden list 
	if(S_ISDIR(mip->INODE.i_mode) != 0){ // non-zero if not a directory 
		lsDir(mip);	
	}
	else {
		lsFile(mip, pathname);
	}

	//since we've loaded an inode into memory, we may put it back into disk.
	iput(mip);	
}

void lsDir(MINODE *mip){
	MINODE *tempMip;
	char tempName[256];
	char tempBuf[1024];
	int i;
	i = 0;

	//Go through all 12 direct blocks for record info.	
	while(i < 12 && mip->INODE.i_block[i] != 0){
		get_block(mip->dev, mip->INODE.i_block[i], tempBuf);
		dp = (DIR *) tempBuf;
		cp = (char *) dp;

		do{
			if(dp->inode == 0){
				break;
			}

			strcpy(tempName, "");
			strncpy(tempName, dp->name, dp->name_len);		
			tempName[dp->name_len] = '\0';
			
			if(DEBUG){
				printf("inside of lsDir\n");
				printf("|inode = %d rec_len = %d name_len = %d name = %s", dp->inode, dp->rec_len, dp->name_len, tempName);	
				getchar();	
			}

			//make a temporary MINODE to pass into lsFile
			tempMip	= iget(mip->dev, dp->inode);	
			lsFile(tempMip, tempName);
			iput(tempMip);

			cp += dp->rec_len;
			dp = (DIR *) cp;
		}while(cp < tempBuf + BLKSIZE);
		
		if(DEBUG){
			printf("|\n");
		}
		
		i++;
	}		
}

void lsFile(MINODE *mip, char *pathname){	
	int i;
	char perms[3] = {'x', 'w', 'r'};
	char time[256];
	printf("%2d ", mip->ino);	//inode number of file.
	
	/******************
	 * Regular file (-)
	 * Directory files(d)
	 *******************/

	/**Special Files***
	 * Block file (b)
	 * Character device file(c)
	 * Named pipe file or just a pipe file(p)
	 * Symbolic link file (l)
	 * Socket file (s)
	 *******************/

	//linux.die.net/man/2/stat
	if(S_ISREG(mip->INODE.i_mode)){
		putchar('-');	
	}	
	else if(S_ISDIR(mip->INODE.i_mode)){
		putchar('d');
	}
	else if(S_ISLNK(mip->INODE.i_mode)){
		putchar('l');
	}
	else if(S_ISBLK(mip->INODE.i_mode)){
		putchar('b');
	}
	else if(S_ISCHR(mip->INODE.i_mode)){
		putchar('c');
	}
	else if(S_ISFIFO(mip->INODE.i_mode)){
		putchar('p');
	}
	else if(S_ISSOCK(mip->INODE.i_mode)){
		putchar('s');
	}

	//ip->imode = 0x41ED -- this is an example from KC wangs notes. this is 
	//4 = 0100 for file type bit
	//1 = 0001, E = 1110, D = 1101 so the permissions are (whatever 4 is)rwxr-xr-x

	for(i = 8; i >= 0; --i){
		if(mip->INODE.i_mode & (1 << i)){
			putchar(perms[i % 3]);
		}
		else{
			putchar('-');
		}
	}

	//gets rid of \n char
	strncpy(time, (char *) ctime(&mip->INODE.i_ctime), strlen((char *) ctime(&mip->INODE.i_ctime)) - 1);	
	time[strlen((char *) ctime(&mip->INODE.i_ctime)) - 1] = '\0';

	//links gid uid size date name
	printf(" %d %d %d %d %s %s\n", 
			mip->INODE.i_links_count, 
			mip->INODE.i_uid, 
			mip->INODE.i_gid, 
			mip->INODE.i_size, 
			time,
			pathname
				);
}
