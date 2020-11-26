#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
/*
 * Algorithm

qntArqCopiados = 0
qntDirCopiados = 0
ttlBytesCopiados = 0

copiaDir(origem, destino)
    se não existe(origem)
        break

    cntArqInterno = qntArq(origem)
    cntDirInterno = qntDir(origem)

    for cntArqInterno até 0
        ttlBytesCopiados += copiaArq(arquivo[cntArq])
        qntArqCopiados++
        imprime origem/arquivo[cntArqInterno]->destino/arquivo[cntArqInterno]

    for cntDirInterno até 0
        copiaDir(diretorio[cntDirInterno])
        qntDirCopiados++
        imprime origem/arquivo[cntDirInterno] -> destino/arquivo[cntDirInterno]


imprime copiados qntDirCopiados, qntArqCopiados e ttlBytesCopiados

*/

/* File descriptors to fileIn (origin) and fileOut (copy) */
int fdIn,fdOut;

/* Info about file in*/
//mode_t modeFileIn;                  // file access mode

unsigned int cntCopiedFiles = 0;
unsigned int cntCopiedDirectories = 0;
unsigned long int cntCopiedBytes = 0;

int treeCopy(char *pathIn, char *pathOut){
    DIR *dirIn;
    struct dirent **listNames;
    int n;

    if((dirIn = opendir(pathIn)) == NULL){
        printf("treecopy: Could not open \"%s\" - %s\n", pathIn, strerror(errno));

        exit(EXIT_FAILURE);
    } else{
        printf("treecopy: Ok - %p\n", dirIn);
    }
    n = scandir(pathIn, &listNames, NULL, alphasort);
    printf("%d\n", n);

    while(n--){
        printf("name: %s\t type: %c\n", listNames[n]->d_name, listNames[n]->d_type);
    }

}

/* Main - treecopy */
int main(int argc, char *argv[]){

    /* TEMPORARY */
    argc = 3;
    strcpy(argv[1], "origTest");
    strcpy(argv[2], "destTest");

    /* Input and output directories pathname*/
    char *pathIn = argv[1];     // origin
    char *pathOut = argv[2];    // copy

    /* Check if the treecopy call has right parameters.
     * If not, print appropriate error message and exit */
    if(argc != 3){
        printf("treecopy: Number of arguments incorrect.\n");
        printf("\tuse: \"treecopy directoryInput directoryOutput\"\n");
        exit(EXIT_FAILURE);
    }

    treeCopy(pathIn, pathOut);

    exit(EXIT_SUCCESS);
}
