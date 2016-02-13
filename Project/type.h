/*	type.h for CS360 Project             */

#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <linux/fs.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 
char *cp;

//#define BLOCK_SIZE        1024
#define BLKSIZE           1024

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777 
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define READY             2
#define KILLED            3

// Table sizes
#define NMINODE           50
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50

// Open File Table
typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
} OFT;

// PROC structure
typedef struct Proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  int   status;

  struct Minode *cwd;
  OFT   *fd[NFD];

  struct Proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
} PROC;
      
// In-memory inodes structure
typedef struct Minode{		
  INODE INODE;               // disk inode
  int   dev, ino;

  int   refCount;
  int   dirty;
  int   mounted;
  struct Mount *mountptr;
  char     name[128];           // name string of file
} MINODE;

// Mount Table structure
typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        struct Minode *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
} MOUNT;

//for mount
MOUNT mounttab[5];

//processes
PROC proc[NPROC];

//for tokenize
char name[64];
char names[64][64];

//for iget
MINODE minode[100];
int inode_start; //for iput

//for search
//int *dev;

//for debugging
int DEBUG;

//for getino
PROC *running;

PROC P0, P1;
MINODE *root; 
MINODE *mip;
int ino;
int bno;
int fd;
int dev;
char *disk;
//char buf[1024];

//for ialloc
int imap;
int ninodes;

//for balloc
int bmap;
int nblocks;

//prototypes

//utilities
int get_block(int fd, int blk, char buf[]);
int put_block(int fd, int blk, char buf[]);

void tokenize(char *pathname);

int getino(int *dev, char *pathname);

void init();

void super();
void groupDescriptor();
void imapBlock();
void bmapBlock();

int test_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int incFreeInodes(int dev);
int intFreeBlocks(int dev);
int decFreeInodes(int dev);
int decFreeBlocks(int dev);

MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int findmyname(MINODE *parent, int myino, char *myname);
int findino(MINODE *mip, int *myino, int *parentino);

int ialloc(int dev);
int balloc(int dev);
int idealloc(int dev, int ino);
int bdealloc(int dev, int bno);
MINODE *mialloc();
int midealloc(MINODE *mip);

/***********level 1************/
void mount_root(int dev);

//ls.c
void ls(char *pathname);
void lsFile(MINODE *mip, char *pathname);
void lsDir(MINODE *mip);

//cd.c
void cd(char *pathname);

//pwd.c
void pwd();

//mkdir.c
void make_dir(char *pathname);
int mymkdir(MINODE *pip, char *name);
int enter_name(MINODE *pip, int myino, char *myname);

//rmdir.c
int rmdir(char *pathname);
int rm_child(MINODE *parent, char *name);

//creat.c
void creat_file(char *pathname);
int my_creat(MINODE *pip, char *name);
int enter_file_name(MINODE *pip, int myino, char *myname);

//link.c
void link(char *oldFileName, char *newFileName);

//symlink.c
void symlink(char *oldFileName, char *newFileName);

//unlink.c
void unlink(char *fileName);
void trunctuate(MINODE *mip);

//touch.c
void touch_file(char *fileName);

//chmod.c
void chmod_file(char *fileName, char *permissions);

//chown.c
void chown_file(char *fileName, char *uid);

//stat.c
void stat_file(char *pathname, struct stat *statBuf);

//rm.c
int rm_file(char *pathname);

//menu.c
void menu();
int findCmd(char *cname, char **cmdList);

/************** Level 2 ***************/

//open.c
int open_file(char *pathname, char *mode);

//close.c
int close_file(int fd);

//pfd.c
int pfd();

//lseek.c
int lseek_file(int fd, int position);

//read.c
int read_file(int fd, char buf[], int bytes);

//write.c
int write_file(int fd, char buf[], int bytes);

//cat
void cat_file(char *pathname);

//cp
void cp_file(char *src, char *dst);

//mv
void mv_file(char *src, char *dst);

#endif
