#include <stdio.h>
#include <stdlib.h>


/*
 * Making a frequency table of the unsigned chars in
 * infilep. The besides the filehandle, the function
 * also takes a pointer to an int array of size 256.
 */
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


/*
 * Huffman coding
 *
 * This program can create Huffman code tables and
 * use them to compress/decompress input files.
 *
 *
 */
int main(int argc, char ** argv) {

    /*
     * File- and input arg handling
     */

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




    /*
     * making frequency table
     */

    freqAnalysis(infilep, freqTable);

    // visualizing freq analysis
    /*
    for (int iii = 0; iii < 256; iii++){
        printf("%c = %i = %i\n", iii, iii, freqTable[iii]);
    }*/


    /*
     * creating Huffman table
     */

    // use priority queue to make tree


    // read out huffman codes into array





    return 0;
}