/****************************************************************************
*              YOUR_INITIALS testing ext2 file system                       *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"
#include "cd_ls_pwd.c"
#include "symlink.c"
#include "write_cp.c"
#include "mount_umount.c"
#include "misc1.c"

char gpath[128]; // global for tokenized components
char *pname[32];  // assume at most 32 components in pathname
int   n;         // number of component strings
char pathname[128];

int fds;

MINODE *iget();

int fs_init()
{
  int i,j;
  for (i=0; i<NMINODE; i++) // initialize all minodes as FREE
    minode[i].refCount = 0;

  for (i=0; i<NMTABLE; i++) // initialize mtables as FREE
    mtable[i].dev = 0;

  for (i=0; i<NOFT; i++) // initialize ofts as FREE
    oft[i].refCount = 0;

  for (i=0; i<NPROC; i++){ // initialize PROCs

    proc[i].status = READY; // ready to run

    proc[i].pid = i; // pid = 0 to NPROC-1

    proc[i].uid = i; // P0 is a superuser process

    for (j=0; j<NFD; j++)
      proc[i].fd[j] = 0; // all file descriptors are NULL
  
    proc[i].next = &proc[i+1]; // link list
  }

  proc[NPROC-1].next = &proc[0]; // circular list

  running = &proc[0]; // P0 runs first
}

int mount_root(char *rootdev)
{
  printf("ROOTDEV = %s\n\n", rootdev);
  int i;
  MTABLE *mp;
  SUPER *sp;
  GD *gp;
  char buf[BLKSIZE];

  globalDev = open(rootdev, O_RDWR);

  if (globalDev < 0){
    printf("panic : cant open root device\n");
    exit(1);
  }

  /* get super block of rootdev */
  get_block(globalDev, 1, buf);
  sp = (SUPER *)buf;

  /* check magic number */
  if (sp->s_magic != SUPER_MAGIC){
    printf("super magic=%x : %s is not an EXT2 filesys\n",
    sp->s_magic, rootdev);
    exit(0);
  }

  // fill mount table mtable[0] with rootdev information
  mp = &mtable[0]; // use mtable[0]
  mp->dev = globalDev;

  // copy super block info into mtable[0]
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;
  strcpy(mp->devName, rootdev);
  strcpy(mp->mntName, "/");
  get_block(globalDev, 2, buf);
  gp = (GD *)buf;
  bmap = mp->bmap = gp->bg_block_bitmap;
  imap = mp->imap = gp->bg_inode_bitmap;
  iblock = mp->iblock = gp->bg_inode_table;
  printf("bmap=%d imap=%d iblock=%d\n", bmap, imap, iblock);

  // call iget(), which inc minode’s refCount
  
  root = iget(globalDev, 2); // get root inode
  mp->mntDirPtr = root; // double link
  root->mptr = mp;

  // set proc CWDs
  for (i=0; i<NPROC; i++) // set proc’s CWD
    proc[i].cwd = iget(globalDev, 2); // each inc refCount by 1

  printf("mount : %s mounted on / \n", rootdev);
  return 0;
}

int quit()
{
  int i;

  for (i=0; i<NMINODE; i++)
  {
    MINODE *mip = &minode[i];
    if (mip->refCount && mip->dirty)
    {
      mip->refCount = 1;
      iput(mip);
    }
  }

  exit(0);
}

int main(int argc, char *argv[ ])
{
  int ino, linefd, linep, lineposition, linenbytes, lineuid, mode;
  char buf[BLKSIZE];
  char line[128], cmd[32], obj[32];

  char pathname2[128];

  char diskname[128];

  char *disk = rootdev;

  if (argc > 1)
  {
    disk = argv[1];

    rootdev = disk;
  }

  printf("\n\n\nIf you've gotten here, congratulations! The program has compiled.\n\n");
  printf("There are several utility functions that are not yet complete-\nwhich means beyond this point, you won't be able to properly load up the filesystem.\n\n");
  printf("Take a look around at the comments to figure out what's missing-\nwhere you can find it, and what you're going to have to do yourself.\n\n");
  printf("Good luck and take care!\n\n\n\n");

  fs_init();

  mount_root(rootdev);

  while(1){
    printf("P%d running: ", running->pid);
    printf("input command : ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
      continue;
    bzero(pathname, 128);
    sscanf(line, "%s %s %s", cmd, pathname, obj);
    
    if (!strcmp(cmd, "ls"))
      my_ls(pathname);
    if (!strcmp(cmd, "cd"))
      chdir(pathname);
    if (!strcmp(cmd, "pwd"))
      pwd(running->cwd);
    if (!strcmp(cmd, "quit"))
      quit();
    if (!strcmp(cmd, "mkdir"))
      make_dir(pathname);
    if (!strcmp(cmd, "creat"))
      creat_file(pathname);
    if (!strcmp(cmd, "chmod"))
      my_chmod(pathname, obj);
    if (!strcmp(cmd, "utime"))
      my_utime(pathname);
    if (!strcmp(cmd, "chown"))
      my_chown(pathname, obj);
      if (!strcmp(cmd, "rmdir"))
      remove_dir(pathname);
    if(!strcmp(cmd, "unlink"))
      my_unlink(pathname);
    if(!strcmp(cmd, "link"))
      my_link(pathname, obj);
    if(!strcmp(cmd, "symlink"))
      symlink_file(pathname, obj);
    if(!strcmp(cmd, "stat"))
      my_stat(pathname);
    if(!strcmp(cmd, "cp"))
      my_cp(pathname,obj);
    if(!strcmp(cmd, "pfd"))
      pfd();
    if(!strcmp(cmd, "open"))
      open_file(pathname, obj);
    if(!strcmp(cmd, "cat"))
      cat_file(pathname);
    if(!strcmp(cmd, "read"))
    read_file(pathname, obj);
    if(!strcmp(cmd, "write"))
      write_file(pathname, obj);
      if(!strcmp(cmd, "close"))
      close_file(pathname);
      if(!strcmp(cmd, "mount"))
      mount(pathname,obj);
  }
}
