#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/* Block size at read/write once */
#define SZ_BLOCK 4000

/* File descriptors to fileIn (origin) and fileOut (copy) */
int fdIn,fdOut;

/* Info about file in*/
mode_t modeFileIn;                  // file access mode
unsigned long int lenFileIn;        // fileIn length

/* Maintenance variables of the copy algorithm */
unsigned long int sizeBlock = 0;    // current block size to be read/write
unsigned long int qntBlocks = 0;    // number of blocks to be read/write
unsigned long int cntRWBlocks;      // counter of read/write blocks
unsigned long int bytesLeft;        // amount of bytes remaining. Useful to calc sizeBlock and qndBlocks
unsigned long int cntBytes = 0;     // counter of byte read from fileIn to fileOut


/* Get 'fileName' file status.
 * Set length and access mode of the file.
 * Return 0 if success, -1 if error */
int getFileStat(char *fileName){

    struct stat fileStat;              // struct to salve file info
    if(stat(fileName, &fileStat) <0)   // get fileName info
        return -1;

    modeFileIn = fileStat.st_mode;     // set file mode
    lenFileIn = fileStat.st_size;      // set length file
    return 0;
}

/* Calc number of blocks necessary to copy algorithm.
 * Here, bytesLeft is bytes remaining from file, qntBlocks is number of blocks, sizeBlock is current block size.
 * At the end, bytesLeft the remaining amount o bytes. */
void calcBlocks(){

    /* Maintenance variables of the calcBlocks algorithm */
    float div = 1;                            // block calculation divider. Lowest valid value is 1.
    float szBlockFloat = (float)SZ_BLOCK;     // aux to SZ_BLOCK in float

    /* If qntBytes is less than 1, ends calc. Here, bytesLeft is 0 */
    if(bytesLeft < (unsigned long int)div)      // using casting to adjust variables to compare them precisely
        exit(EXIT_SUCCESS);

    /* Increments/Multiplies the divisor by 10 until SZ_BLOCK*/
    while((SZ_BLOCK > ((unsigned long int)div)) && (bytesLeft <= (unsigned long int)(szBlockFloat / div)))
        div *= 10;

    /* Update qntBlocks, sizeBlock and bytesLeft */
    /* Here, the divisor still lees than SZ_BLOCK. In this case bytesLeft
     * is less than the smallest possible block ((SZ_BLOCK * 10) / div)*/
    if(SZ_BLOCK >= (unsigned long int)div){
        sizeBlock =  (unsigned long int)(szBlockFloat / div);   // new and most suitable block size
        qntBlocks = bytesLeft / sizeBlock;                      // number of blocks adapted to the new block size
        bytesLeft =  (bytesLeft % sizeBlock);                   // remaining bytes of division with new block size
    } else{                      // In this case bytesLeft is greater than the smallest possible block, 1*/
        sizeBlock = bytesLeft;   // now, sizeBlock is the size of bytesLeft
        qntBlocks = 1;           // remaining form just one block
        bytesLeft =  0;          // there are not remaining bytes
    }
}

/* Main - filecopy */
int main(int argc, char *argv[]){

    /* Input and output file names*/
    char *fileIn = argv[1];     // origin
    char *fileOut = argv[2];    // copy

    /* Check if the filecopy call has right parameters.
     * If not, print appropriate error message and exit */
    if(argc != 3){
        printf("filecopy: Number of arguments incorrect.\n");
        printf("\tuse: \"filecopy fileInput fileOutput\"\n");
        exit(EXIT_FAILURE);
    }

    /* Open an existing file with permission read only
     * or print appropriate error message and exit */
    if((fdIn = open(fileIn, O_RDONLY)) < 0){
        printf("filecopy: Could not open \"%s\" - %s\n", fileIn, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Catch fileIn stats and set its length and mode.
    * or print appropriate error message and exit */
    if((getFileStat(fileIn)) < 0){
        printf("filecopy: Could not read \"%s\" - %s\n", fileIn, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Create an output file with same permissions at the fileIn (modeFileIn),
     * or print appropriate error message and exit */
    if((fdOut = creat(fileOut, modeFileIn)) < 0) {
        printf("filecopy: Could not create \"%s\" - %s\n", fileOut, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Copy algorithm */
    bytesLeft = lenFileIn;      // fileIn length
    while(bytesLeft > 0){

        calcBlocks();

        char buf[sizeBlock];    // buffer to copy. Its size may vary
        cntRWBlocks =0;         // reset counter of read/write block

        while(cntRWBlocks < qntBlocks){

            /* Read sizeBlock bytes from fileIn to buf.
             * If error, print appropriate message and exit */
            if((read(fdIn, &buf, sizeBlock)) < 0){
                printf("filecopy: Error copying from \"%s\" - %s\n", fileIn, strerror(errno));
                exit(EXIT_FAILURE);
            }

            /* Write sizeBlock bytes of buffer to fileOut.
             * If error, print appropriate message and exit */
            if(write(fdOut, &buf, sizeBlock) < 0){
                printf("filecopy: Error copying to \"%s\" - %s\n", fileOut, strerror(errno));
                exit(EXIT_FAILURE);
            }

            cntRWBlocks++;      // increment counter of read/write blocks
        }

        /* Update cntBytes with number of bytes read/write in this iteration */
        cntBytes += (sizeBlock * qntBlocks);
    }

    /* Closing fileIn and fileOut file descriptors */
    close(fdIn);
    close(fdOut);

    /* Compare bytes read from fileIn to fileOut with fileIn length.
     * If equals, print number of the copied bytes. If not, print appropriate message. */
    if(cntBytes == lenFileIn)
        printf("\nfilecopy: %lu bytes were copied from \"%s\" to file \"%s\"\n", cntBytes, fileIn, fileOut);
    else
        printf("filecopy: Unexpected error when copying file");

    exit(EXIT_SUCCESS);
}
