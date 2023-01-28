/************* cd_ls_pwd.c file **************/

#ifndef __CDLSPWD_C__
#define __CDLSPWD_C__

#include "type.h"
#include "util.c"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";
char ftime[256];

int chdir(char *pathname)   
{
  printf("under construction READ textbook HOW TO chdir!!!!\n");
  // READ Chapter 11.7.3 HOW TO chdir
//   (1). int ino = getino(pathname); // return error if ino=0
// (2). MINODE *mip = iget(dev, ino);
// (3). Verify mip->INODE is a DIR // return error if not DIR
// (4). iput(running->cwd); // release old cwd
// (5). running->cwd = mip;
  int ino = getino(pathname);
  MINODE *mip = iget(globalDev, ino);
  if(S_ISDIR(mip->INODE.i_mode)){
    iput(running->cwd);
    running->cwd = mip;
  }
  else{
    printf("ERROR: Not a directory!\n");
    iput(mip);
    return -1;
  }

  return 1;
}

int ls_file(MINODE *mip, char *name)
{
    //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  //u16 modeInfo = mip->INODE.i_mode;
  //gets the Inode info to a temporary value to not change it
  INODE *temp = &mip->INODE; 
  //The [dev, ino] will be printed after
  // printf("PAST node\n");
  //checks if its a file
  if (S_ISREG(temp->i_mode)){
    printf("%c", '-');
  }
  //checks if its a directory
  if (S_ISDIR(temp->i_mode)){
    printf("%c", 'd');
  }
  // checks if it has a lnk
  if (S_ISLNK(temp->i_mode)){
    printf("%c", 'l');
  }
  //gets the modes based off t1 and t2
  for (int i=8; i >= 0; i--){
  if (temp->i_mode & (1 << i)) // print r|w|x
  printf("%c", t1[i]);
  else
  printf("%c", t2[i]); // or print -
  }
  // printf("%4d ",sp->st_nlink); // link count
  // printf("%4d ",sp->st_gid); // gid
  // printf("%4d ",sp->st_uid); // uid
  // printf("%8d ",sp->st_size); // file size

  //prints the links, gid, and id
  printf("%4d ", temp->i_links_count); // link count numbers of ways that I can access
  printf("%4d ", temp->i_gid); // gid
  printf("%4d   ", temp->i_uid); // uid
  
  //Get the time that it was made with temps mtime
  strcpy(ftime, ctime(&(temp->i_mtime)));
  ftime[strlen(ftime)-1] = 0;
  //print the time 
  printf("%s", ftime);
  //prints size of temp
  printf("%8d  ", temp->i_size); // file size
  //prints the name and checks if it has a link if it does we print that
  printf("%s ", name);
  if (S_ISLNK(temp->i_mode)){
    printf("->%s ", temp->i_block);
  }
  //we print the dev and ino
  printf("[%d %d]", mip->dev, mip->ino );
  printf("\n");
  //return 1;
}

// All this does is it gets the imip info and prints it (inode, reclen, namelen, name)
int ls_helper(MINODE *mip){
  char buf[BLKSIZE], temp[BLKSIZE];
  DIR *dp;
  char *cp;
  MINODE *dip;
  printf("ls_dir: Block information!\n");
  get_block(mip->dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  printf("INODE   Rec_len   Name_len  Name\n");
  while (cp < buf + BLKSIZE){
    //printf("We are in\n");
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     printf("%4d  %4d\t%4d\t     %s\n", dp->inode, dp->rec_len  , dp->name_len, temp);
     //printf("%s  ", temp);
     dip = iget(globalDev,dp->inode);
     //ls_file(dip,temp);
     iput(dip);
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");

  return 1;
}

int ls_dir(MINODE *mip)
{
  

  //INODE temps = mip->INODE;
  //calls the mip info
  ls_helper(mip);
  //set variables
  char buf[BLKSIZE], temp[BLKSIZE];
  DIR *dp;
  char *cp;
  MINODE *dip;
  printf("ls_dir: Long format!\n");
  //we get the vlock
  get_block(mip->dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
    //printf("We are in\n");
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     //printf("%4d  %4d  %4d   %s\n", dp->inode, dp->rec_len  , dp->name_len, temp);
     //printf("%s  ", temp);
     dip = iget(globalDev,dp->inode);
     //call the file for all of the items in the directory one by one
     ls_file(dip,temp);
     iput(dip);
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");

  return 1;
}

int my_ls(char *pathname)  
{
  //we check if there is a pathname if not we just run lsdir on current file
  printf("ls %s\n", pathname);
  printf("YOU do it for ANY pathname\n");
  if (!strcmp(pathname, "")){
    ls_dir(running->cwd);
  }
  else{
    //would be the case that we do have a pathname
    //ls_dir(pathname);
    // we check if ino exists first
    int ino = getino(pathname);
    if (ino <= 0){
      printf("ERROR: there was no inode with this pathname\n");
      return -1;
    }
    MINODE *mip = iget(globalDev, ino);
    //printf("%d\n\n", mip->INODE.i_mode);
    //checks if its a file
    if (S_ISREG(mip->INODE.i_mode)){
      printf("ERROR: This is a file!\n");
      return -1;
    }

    else{
      // iput(running->cwd);
      // running->cwd = mip;
      printf("IN else %d", mip->ino);
      ls_dir(mip);
    }
  }
  
  return 1;
}

char *rpwd(MINODE *wd)
{
 // printf("pwd: ");
//   (1). if (wd==root) return;
// (2). from wd->INODE.i_block[0], get my_ino and parent_ino
// (3). pip = iget(dev, parent_ino);
// (4). from pip->INODE.i_block[ ]: get my_name string by my_ino as LOCAL
// (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
// (6). print "/%s", my_name;
    if(wd == root) 
      return " ";

  int ino;
  MINODE* mip = wd;
  int *childino;
  int parentino;
  char myName[50];

  parentino = findino(mip, &childino);
  //printf("pC: %d %d\n", parentino, childino);
  MINODE *p_mip = iget(globalDev, parentino);
  findmyname(p_mip, childino, myName);
  //printf("nam:%s", myName);
  //printf("inonum: %d", p_mip->ino);
  rpwd(p_mip);
  printf("/%s", myName);
  //free(myName);
  return "";


}

char *pwd(MINODE *wd)
{
  //printf("pwd: ");
  
  if (wd == root){
    printf("pwd: ");
    printf("/\n");

  }
  else{
    printf("pwd: ");
    rpwd(wd);
    printf("\n");
  }
// pwd start:
//pwd(running->cwd);
}

#endif