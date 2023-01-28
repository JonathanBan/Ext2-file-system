/************* write_cp.c file **************/

#ifndef __WRITECP_C__
#define __WRITECP_C__

#include "read_cat.c"
#include "link_unlink.c"

int write_file(char* fds, char *buf)
{
    int fd = atoi(fds);

    // preparations:
    // ask for a fd and a text string to write
    int bytes;

    // verify fd is indeed opened for WR or RW or APPEND mode
    OFT * oftp = running->fd[fd];
    printf("the mode is %d\n", oftp->mode);
    if(oftp->mode == 0)
    {
        printf("Cannot write, fd = %d is opened for read mode\n", fd);
        return -1;
    }

    if(oftp == 0)
    {
        printf("fd %d is not opened\n", fd);
        return -1;
    }

    bytes = strlen(buf);

    return (my_write(fd, buf, bytes));
}

int my_write(int fd, char buf[], int nbytes)
{
    int lbk, startByte, blk, remain;

    int ibuf[256]; int jbuf[256]; int kbuf[256];
    //MINODE * mip = running->fd[fd]
    // //MINODE * mip = running->fd[fd]->minodeptr;
    char* tmp[1024];
    OFT * oftp = running->fd[fd];
    MINODE* mip = oftp->minodeptr;
    
    char *cq= buf;

    char buf2[BLKSIZE];

    while(nbytes > 0)
    {
        // compute LOGICAL BLOCK (lbk) and the startByte in that lbk;

        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;
        printf("startbyte: %d\n", startByte);
        // I only show how to write DIRECT data blocks, you figure out how to 
        // write indirect and double-indirect blocks.

        if(lbk < 12)
        {
            printf("In first\n");
            if(mip->INODE.i_block[lbk] == 0) // if no data block 
            {
                mip->INODE.i_block[lbk] = balloc(mip->dev); // allocate a block
            }
            blk = mip->INODE.i_block[lbk]; // blk should be a disk block now
            printf("balloc: bno = %d\n", blk);
            //get segfault here
            //printf("allocating direct block blk = %d\n", blk);
        }
        else if (lbk >= 12 && lbk < 256 + 12) // INDIRECT blocks
        {
            printf("in else if\n");
            // HELP INFO:;
            if(mip->INODE.i_block[12] == 0)
            {
                //allocate a block for it;
                int block = mip->INODE.i_block[12] = balloc(mip->dev);
                //zero out the block on disk
                //maybe turuncate
                get_block(globalDev, mip->INODE.i_block[12], buf2);
                int size_buf = sizeof(buf2);
                memset(buf2, '\0', size_buf);
               put_block(globalDev, mip->INODE.i_block[12], buf2);
            }
            printf("after 12 block\n");

            // get i_block[12] into an int ibuf[256]

            get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
            printf("past else if getblock\n");
            blk = ibuf[lbk-12];

            if(blk == 0)
            {
                // allocate a disk block
                ibuf[lbk-12] = balloc(mip->dev);
                // record it in i_block[12];
                blk = ibuf[lbk-12] = balloc(mip->dev);
                put_block(mip->dev, mip->INODE.i_block[12], ibuf);
            }

            

        }
        else // double indirect blocks
            {
                // double indirect blocks

                lbk -= ( 12+256); // decrement lbk

                // get_iblock[13] into an int jbuf[256]
                get_block(mip->dev, mip->INODE.i_block[13], jbuf);

                // got i_block-13 in, which have 256 numbers

                int chunk = lbk / 256;
                get_block(mip->dev, jbuf[chunk], kbuf);
                //check if its 0 when block just like in if
                //Mailman's algorithm 
                blk = kbuf[lbk % 256];
            }
            //from book
            get_block(mip->dev, blk, tmp);   // read disk block into wbuf[ ]  
            printf("past getblock after statements\n");

            
            char *cp = tmp + startByte;      // cp points at startByte in wbuf[]
            remain = BLKSIZE - startByte;
            printf("going into while loop\n");
            while(remain > 0) // write as much as remain allows
            {
                *cp++ = *cq++;
                nbytes--; remain--;
                oftp->offset++;

                if(oftp->offset > mip->INODE.i_size)
                {
                    mip->INODE.i_size++; //inc file size(if offset > filesize)

                }

                if(nbytes <= 0)
                    break;

            }
            printf("tmp: %s\n", tmp);
            put_block(mip->dev, blk, tmp);

            }
            
            mip->dirty = 1; // mark mip dirty for iput()
            printf("After dirty!\n");
            return nbytes;

}

int my_cp(char *src, char* dest)
{   
    char mybuf[1024], dummy= 0;
    int n;
    printf("In cp!\n");
    int fd = open_file(src, "0");
    printf("after first open\n");
    pfd();
    //creat_file(dest);
    printf("after creat\n");
    pfd();
    
    int gd = open_file(dest, "1");//changed this
    char* fds=  (char*)malloc(20);
    sprintf(fds, "%d", fd);
    char* gds=  (char*)malloc(20);
    printf("ha: %s %s\n", src, dest);
    sprintf(gds, "%d", gd);
    printf("THE GD: %d\n\n", gd);
    printf("past opened!\n");
    if(fd == -1){
        printf("File named has failed to open!\n");
        
        close_file(fds);
        close_file(gds);
        return -1;
    }
    if (gd== -1){
        printf("The destination file has failed to open!\n");
        close_file(fds);
        close_file(gds);
        return -1;
    }
    printf("fd: %d\n", fd);
    printf("GOING INTO the while loop!\n\n");
    printf("buf: %s\n", mybuf);
    //pfd();
     pfd();
   // memset(buf, '\0', BLKSIZE);
    // n = my_read(fd,buf, 1024);
    while( n = my_read(fd, mybuf, 1024)){
        printf("buf: %s\n", mybuf);
      printf("n: %d\n\n", n);
       mybuf[n] = 0;             // as a null terminated string

       my_write(fd, mybuf, n);   //<=== THIS works but not good
    //   spit out chars from mybuf[ ] but handle \n properly;

   } 

    // while( n = my_read(fd, buf, 1024) ){//this is the problem
    //     buf[n]= 0;
    //     printf("IN THE WHILE LOOP buf %s\n", buf);
    //     printf("n: %d\n",n);
    //    buf[n]=0;
    //    my_write(gd, buf, n);  // notice the n in write()
    //   // memset(buf, '\0', BLKSIZE);
    // }
    printf("closed FILE \n\n");
    close_file(fds);
    close_file(gds);

    return 1;
}

#endif