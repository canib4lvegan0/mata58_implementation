#include "utility.h"

/* Block size at read/write once */
#define SZ_BLOCK 4000

/* Info about file in*/
mode_t modeFileIn;                  // file access mode
unsigned long int lenFileIn;        // fileIn length

/* Maintenance variables of the copy algorithm */
extern long int sizeBlock;    // current block size to be read/write
extern unsigned long int qntBlocks;    // number of blocks to be read/write
extern unsigned long int cntRWBlocks;      // counter of read/write blocks
extern unsigned long int bytesLeft;        // amount of bytes remaining. Useful to calc sizeBlock and qndBlocks
extern unsigned long int cntBytes;     // counter of byte read from fileIn to fileOut

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
