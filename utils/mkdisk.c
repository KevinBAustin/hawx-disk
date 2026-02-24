#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BSIZE 1024

int main(int argc, char **argv)
{
    int fd = -1;
    int blocks = 0;
    char buf[BSIZE];

    // check for proper usage
    if (argc != 3) {
        fprintf(stderr, "Usage: mkdisk <diskname> <blocks>\n");
        exit(1);
    } 

    // attempt to open the file for writing
    if ((fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror("open");
        exit(1);
    }

    // generate a block of zeroes
    memset(buf, 0, BSIZE);

    // get the number of blocks
    blocks = atoi(argv[2]);

    // write the blocks to the file
    for (int i = 0; i < blocks; i++) {
        if (write(fd, buf, BSIZE) != BSIZE) {
            perror("write");
            exit(1);
        }
    }

    close(fd);
}
