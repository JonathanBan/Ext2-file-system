/************* mkdir_creat.c file **************/

#ifndef __MKDIRCREAT_C__
#define __MKDIRCREAT_C__

#include "type.h"
#include "util.c"

int enter_name(MINODE *pip, int ino, char *name)
{
    //INODE *temps = &pip;
    //printf("minode temps: %d", temps->INODE.i_size);
    char buf[BLKSIZE], *cp;
    int ideal_length,need_length = 0;
    int name_len = strlen(name);
    int remain = 0;
    
    printf("iblock %d\n", pip->INODE.i_block[0]);
    for(int i = 0; i < 12; i++){
        printf("in for loop start\n");
        ideal_length =4*( (8 + name_len + 3)/4 );
        if (pip->INODE.i_block[i] == 0){
            break;
        }
        printf("in for loop \n");
        // need_length =  4*( (8 + n_len + 3)/4 ) 
        
        // get_block(pip->dev, pip->INODE.i_block[i], buf);
        get_block(pip->dev, pip->INODE.i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;
        while (cp + dp->rec_len < buf + BLKSIZE){
        cp += dp->rec_len;
        dp = (DIR *)cp;
        }
        ideal_length = 4*( (8 + dp->name_len + 3)/4 );
        // dp NOW points at last entry in block
        remain = dp->rec_len - ideal_length;
        if (remain >= need_length){
        // enter the new entry as the LAST entry and
        // trim the previous entry rec_len to its ideal_length;
        // need help with this
        printf("int the remain if statement\n");
        dp->rec_len = ideal_length;
        cp += dp->rec_len;
        dp = (DIR *)cp;
        strcpy(dp->name ,name);
        dp->inode = ino;
        dp->name_len = name_len;
        dp->rec_len = remain;
        put_block(globalDev, pip->INODE.i_block[i],buf);
        }
        

    }

    return 0;
}

int my_mkdir(MINODE *pip, char *name)
{
    printf("Int mkdir\n");
    int ino = ialloc(globalDev);
    //int ino = 0;
    printf("Past Ialloc\n");
    int bno = balloc(globalDev);
    char *buf[BLKSIZE];
   // MINODE *mip = iget(globalDev, ino);
    printf("gets past init variables\n");
    MINODE *mip = iget(globalDev, ino);
    INODE *ip = &mip->INODE;

    ip->i_mode = 0x41ED; // 040755: DIR type and permissions
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id
    ip->i_size = BLKSIZE; // size in bytes
    ip->i_links_count = 2; // links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno; // new DIR has one data block
    //ip->i_block[1] to ip->i_block[14] = 0;
    for (int i = 1; i < 15; i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1; // mark minode dirty
    iput(mip); // write INODE to disk

    //bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
    get_block(globalDev, bno,buf);
    printf("past get blocks\n");
    //this makes . and .. entries
   // char buf[BLKSIZE];
   
    DIR *dp = (DIR *)buf;
    // make . entry
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    // make .. entry: pino=parent DIR ino, blk=allocated block
    dp = (char *)dp + 12;
    dp->inode = pip->ino;
    dp->rec_len = BLKSIZE-12; // rec_len spans block
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(globalDev, bno, buf); // write to blk on diks
    printf("Past put block\n");
    enter_name(pip,ino,name);
    
    return 1;
}

int make_dir(char * pathname)
{
    
    char *dir[BLKSIZE];
    char *base[BLKSIZE];
    char *dname[BLKSIZE];
    char *bname[BLKSIZE];

    strcpy(dir, pathname);
    strcpy(base, pathname);

    // dividing pathname to dirname and basename
    strcpy(dname, dirname(dir));
    strcpy(bname, basename(base));

    printf("parent = %s  ", dname);
    printf("child = %s\n", bname);

    int pino = getino(dname);
    printf(" prints: %s %s \n", dname, bname);
    if(pino == 0)
    {
        printf("dirname = %s does not exist.\n", dname);
        return -1;
    }

    MINODE * pmip = iget(globalDev, pino);

    
    // check pmip->INODE is a DIR

    if(!S_ISDIR(pmip->INODE.i_mode)) 
    {
        // if dirname is not a directory
        iput(pmip);
        return -1;
    }
    printf("past dir check\n");
    // check child not exist in parent directory

    if(search(pmip, bname) != 0)
    {
        printf("%s is already exist in %s\n", bname, dname);
        iput(pmip);
        return -1;
    }
    printf("past search!\n");
    printf("baseam: %s", bname);
    // call my_mkdir(pmip, basename) to create a DIR:
    my_mkdir(pmip, bname);
}

int my_creat(MINODE *pip, char *name)
{
    return 1;
}

int creat_file(char *pathname)
{
    return 1;
}

#endif