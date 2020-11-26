#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/* Block size at read/write once */
#define SZ_BLOCK 4000

/* Info about file in*/
mode_t modeFileIn;                  // file access mode
unsigned long int lenFileIn;        // fileIn length

/* Get 'fileName' file status.
 * Set length and access mode of the file.
 * Return 0 if success, -1 if error */
int getFileStat(char *fileName);

/* Calc number of blocks necessary to copy algorithm.
 * Here, bytesLeft is bytes remaining from file, qntBlocks is number of blocks, sizeBlock is current block size.
 * At the end, bytesLeft the remaining amount o bytes. */
void calcBlocks();