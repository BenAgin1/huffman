#include <stdio.h>
#include <stdlib.h>

void freqAnalysis(FILE * infilep, int * freqTable)
{
    int readChar;

    // make frequency table with first arg file
    while ((readChar = fgetc(infilep)))
    {
        if( feof(infilep) )
        {
            break;
        }
        freqTable[readChar]++;
    }
}



int main(int argc, char ** argv) {

    FILE * infilep; /* Pointer to input file */
    int freqTable[256] = { 0 };

    printf("Number of parameters: %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Parameter %d: %s\n", i, argv[i]);
    }

    if (argc <= 1)
    {
        fprintf(stderr, "Usage: huffman <input file name> \n");
        return 0;
    }

    /* Try to open the input file */
    infilep = fopen(argv[1], "r");

    if(infilep == NULL)
    {
        fprintf(stderr, "Could not open input file %s\n", argv[1]);
        return 0;
    }

    freqAnalysis(infilep, freqTable);

    // visualizing freq analysis
    /*
    for (int iii = 0; iii < 256; iii++){
        printf("%c = %i = %i\n", iii, iii, freqTable[iii]);
    }*/

    // find lowest two values in freq array and list of trees

    // build new tree in list of tree either from two values of freq list
    // or 1 value freq list and one value tree list or two value tree list
    // all values from the tree list require rebuilding the tree and then removing
    // the old tree, until just one tree in tree list


    return 0;
}