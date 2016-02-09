#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_3cell.h"
#include "prioqueue.h"
#include "bitset.h"


/* Node of the huffman tree */
typedef struct {
  int value;
  unsigned char character;
} freqChar;

void getFrequency(int *frequency, FILE* file);
int compareTrees(VALUE tree1, VALUE tree2);
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE));
void traverseTree(binaryTree_pos pos, binary_tree* huffmanTree, 
	bitset * navPath, bitset *pathArray[]);
void encodeFile(FILE* encodeThis, FILE* output, bitset *pathArray[]);
void decodeFile(FILE* decodeThis, FILE* output, binary_tree* huffmanTree);
int wrongArgs(void);

int main(int argc, char **argv){

    /*
     * Variables
     */

    int frequency[256];


	/*Check number of command line arguments.*/
	if(argc <= 4){
		return wrongArgs();
	}


	/*
	 * Check and switch for encode / decode
	 */


	int selector;
	char encodeStr[8];
	char decodeStr[8];

	strcpy(encodeStr, "-encode");
	strcpy(decodeStr, "-decode");
	if (!strcmp(argv[1], encodeStr)){
		selector = 1;
	} else if(!strcmp(argv[1], decodeStr)){
		selector = 2;
	}



	/*
	 * Check and open files
	 */
	FILE* freqFilep = fopen(argv[2], "rt");
	if(freqFilep == NULL){
		fprintf(stderr, "Couldn't open frequency file %s\n", argv[2]);
	}

	FILE *infilep;

	if (selector == 1) {
		infilep = fopen(argv[3], "rt");
	} else {
		infilep = fopen(argv[3], "rb");
	}

	//FILE* infilep = fopen(argv[3], "rt");
	if(infilep == NULL){
		fprintf(stderr, "Couldn't open input file %s\n", argv[3]);
	}

	FILE *outfilep;
	if(selector == 1) {
		outfilep = fopen(argv[4], "w");
	} else {
		outfilep = fopen(argv[4], "w");
	}

	//FILE* outfilep = fopen(argv[4], "wb");
	if(outfilep == NULL){
		fprintf(stderr, "Couldn't open output file %s\n", argv[4]);
	}


	switch(selector) {
		
		case 1:
			printf("encoding\n");

			/*
 			 * calculate frequency table
 			 * build huffman tree
 			 * build code table
 			 */


			getFrequency(frequency, freqFilep);
			binary_tree *treeEncode = buildHuffmanTree(frequency, compareTrees);

			bitset *navPath = bitset_empty();
			bitset *pathArray[256];

			traverseTree(binaryTree_root(treeEncode), treeEncode, navPath, pathArray );

			/*
			// test whether we can access the bitset stored in a bitset array
			for (int iii = 0; iii < 256; iii++) {
				printf("%d\n", pathArray[iii]->length);
			}
			*/

			encodeFile(infilep, outfilep, pathArray );

            /*
             * cleanup
             */
            bitset_free(navPath);
            for (int i = 0; i<256; i++){
                bitset_free(pathArray[i]);
            }
            //binaryTree_free(treeEncode);


			break;

        case 2:

			printf("decoding\n");

			/*
 			 * calculate frequency table
 			 * build huffman tree
 			 */

			getFrequency(frequency, freqFilep);
			binary_tree *treeDecode = buildHuffmanTree(frequency, compareTrees);
			bitset *navPath_decode = bitset_empty();
			bitset *pathArray_decode[256];

			traverseTree(binaryTree_root(treeDecode), treeDecode, navPath_decode, pathArray_decode );

			decodeFile(infilep, outfilep, treeDecode);

            /*
             * cleanup
             */
            bitset_free(navPath_decode);
            for (int i = 0; i<256; i++){
                bitset_free(pathArray_decode[i]);
            }
            //binaryTree_free(treeDecode);



			break;
		default:
			fprintf(stderr, "Unknown option selected, exiting program.\n");
			fclose(freqFilep);
			fclose(infilep);
			fclose(outfilep);
			wrongArgs();
	}


	fclose(freqFilep);
	fclose(infilep);
	fclose(outfilep);

	return 0;
}

/*
 * getFrequency - calculates a frequency table on an text input file
 *                using the 256 characters of the extended ASCII table.
 *
 * Parameter:   frequency - pointer to an int array of length 256. Here the
 *                          frequencies will be summed and stored
 *              file      - pointer of type FILE. The input file has to be
 *                          a standard text file.
 */
void getFrequency(int* frequency, FILE* file){
	int finished=0;
	int ch;
	for (ch = 0; ch < 256; ch++){
		frequency[ch] = 1;
	}

	// one count for the EOT char
	frequency[4]++;
	
	while (finished!=1){
		ch = fgetc(file);
		/*Stop reading at EOF.*/
		if (ch == EOF){
			finished=1;
		}
		/*Assuming ASCII, increases the frequency of the character read.*/
		else{
			frequency[ch]++;
		}
	}

	
	/*Modifies frequency values so that no character has value '0'.
	 but still tries to keep the relative frequency.*/
	for(int iii = 0; iii < 256; iii++){
		frequency[iii] *= 1000;
		if(frequency[iii]==0){
			frequency[iii]=1;


		}
		//printf("%c : %d\n", (unsigned char)iii, frequency[iii]);
	}

}

/*
 * compareTrees - is the compare function used in the priorityQueue datatype
 *
 * Paramter:    tree1   - pointer to a binary tree datatype
 *              tree2   - pointer to a binary tree dataype
 *
 * Comments:    This function assumes a freqChar struct to be stored as
 *              the label of the binary tree. The actual comparison is done
 *              between the 'value' of each tree's root.
 */
int compareTrees(VALUE tree1, VALUE tree2){
	freqChar tmp1;
	freqChar tmp2;
	tmp1 = *(freqChar*)binaryTree_inspectLabel(tree1, binaryTree_root(tree1));
	tmp2 = *(freqChar*)binaryTree_inspectLabel(tree2, binaryTree_root(tree2));
	if (tmp1.value > tmp2.value){
		return 0;
	}
	else{
		return 1;
	}
}

/*
 * buildHuffmanTree:    - This function builds a huffman tree from a frequency 
 *                        table
 *
 * Parameter:           frequency   - a pointer to an int array of length 256 
 *                                    that represents an extended ASCII 
 *                                    character frequency table generated by 
 *                                    the function getFrequency
 *                      compare     - pointer to a function that compares the 
 *                                    root label of the two binary trees. This 
 *                                    function will be used as argument for the 
 *                                    priority queue datatype.
 *
 *  The function first makes root/leafs for all 256 characters in the extended 
 *  ASCII table and puts them in a priority queue (datatype pqueue from 
 *  prioqueue.c /.h). Then in a while loop, two elements at a time are removed 
 *  from the priority queue. And linked into a new binary tree root. The label 
 *  of the new tree root contains as value the combined values of the two 
 *  children. This is repeated until just one element is left in the priority 
 *  queue.
 */
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE)){
	pqueue *treebuildingQueue = pqueue_empty (compare);
	//pqueue_setMemHandler(treebuildingQueue, free); // CANT USE THIS
	//PERHAPS THE PRIO QUEUE SHOULD BE BUILT IN A DIFFERENT FUNCTION?
	int chartmp;
	//binary_tree *huffmanTree = binaryTree_create(); // CAN BE REMOVED
	binary_tree *tree1;
	binary_tree *tree2;
	binary_tree *newTree; //ADDED HERE INSTEAD
	//binaryTree__setMemHandler(huffmanTree, free);
	
	/*Create 1 tree for each character and put all of them in a priority queue*/
	for (chartmp = 0; chartmp < 256; chartmp++){
		freqChar *tmp = malloc(sizeof(freqChar));
		tmp->character = chartmp;
		tmp->value = frequency[chartmp];
		//binary_tree* treetmp = binaryTree_create(); NOT NEEDED
		newTree = binaryTree_create(); //USES THIS INSTEAD
		binaryTree_setMemHandler(newTree, free); //ADDED TO TRY TO FIX MEMORY LEAKS
		binaryTree_setLabel(newTree, tmp, binaryTree_root(newTree));
		pqueue_insert(treebuildingQueue, newTree);
	}
	
	/*While priority queue isn't empty take out the two front values and 
	connect these two trees with a new node (tree), put this new combined tree 
	in the queue.*/
	while(!pqueue_isEmpty(treebuildingQueue)){
		
		/*Create new tree with 1 node.*/
		//binary_tree *newTree = binaryTree_create(); // NOT NEEDED
		newTree = binaryTree_create(); // USES THIS INSTEAD
		binaryTree_setMemHandler(newTree, free);
		
		/*Take out the first tree from the queue and save the values on its 
		 label.*/
		tree1 = pqueue_inspect_first(treebuildingQueue);
		freqChar *freqChartmp1 = binaryTree_inspectLabel(tree1, binaryTree_root(tree1)); //binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue))); //TOO LONG
		pqueue_delete_first(treebuildingQueue);
		
		/*When the last tree has been taken out return that tree*/
		if(pqueue_isEmpty(treebuildingQueue)){
			//huffmanTree = tree1;
			binaryTree_free(newTree);
			binaryTree_setMemHandler(tree1, free); // NEW TO TRY
			//free(tree1); SHOULD NOT BE NEEDED?
			pqueue_free(treebuildingQueue); //WANTS TO REMOVE THE QUEUE!!!REMOVES THE TREE!!!
			return tree1; //REPLACES RETURN huffmanTree.
		}
		else{
			
			/*Take out the second tree from the queue and save the values on its
			 label.*/
			tree2 = pqueue_inspect_first(treebuildingQueue);
			freqChar *freqChartmp3 = binaryTree_inspectLabel(tree2, binaryTree_root(tree2));//binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue))); //TOO LONG
			pqueue_delete_first(treebuildingQueue);
			
			/*Initiate and give values to the new node label.*/
			freqChar *freqChartmp2 = malloc(sizeof(freqChar));
			freqChartmp2->value = freqChartmp1->value + freqChartmp3->value;
			freqChartmp2->character = -1;
			
			/*Set label on the new tree and insert a left and right child.*/
			binaryTree_setLabel(newTree, freqChartmp2, 
				binaryTree_root(newTree));
			//binaryTree_insertLeft(newTree, binaryTree_root(newTree)); IS NOT NEEDED SOURCE OF MEMORY LEAK
			//binaryTree_insertRight(newTree, binaryTree_root(newTree)); IS NOT NEEDED SOURCE OF MEMORY LEAK
			
			/*Set the two trees from the queue as right/left child on the 
			 new node.*/
			newTree->root->rightChild = tree1->root;
			newTree->root->leftChild = tree2->root;
			tree1->root->parent = newTree->root; // ADDED THESE TWO
			tree2->root->parent = newTree->root; // ADDED
			
			if(binaryTree_root(newTree) == binaryTree_parent(tree1, binaryTree_root(tree1))){
				//printf("HEJ\n");
			}
			/*Insert the new tree in the queue.*/
			pqueue_insert(treebuildingQueue, newTree);
			
			free(tree2); //MIGHT NEED THESE
			free(tree1); //
		}
	}
	pqueue_free(treebuildingQueue);
	return 0;
}

/*
 * traverseTree - function that traverses a binary tree
 *
 * Parameter:   pos     - position where to start the traversal
 *              tree    - pointer to binary tree to traverse
 *
 * This function expects the leafs of the tree to have labels
 * of type freqChar. It will print out both charachter and value
 * of each leaf. Traversal is pre-order.
 */
void traverseTree(binaryTree_pos pos, binary_tree *huffmanTree, 
	bitset * navPath, bitset *pathArray[]){
	int length = navPath->length;
	
	if(binaryTree_hasLeftChild(huffmanTree, pos)){
		bitset *newPath = bitset_empty();
		for (int iii = 0; iii < length; iii++ ){
			bitset_setBitValue(newPath, iii, bitset_memberOf(navPath,iii));
		}
		bitset_setBitValue(newPath, length, 0);
		traverseTree(binaryTree_leftChild(huffmanTree, pos), huffmanTree, 
			newPath, pathArray);
		//bitset_free(newPath);
	}
	if(binaryTree_hasRightChild(huffmanTree, pos)){
		bitset *newPath = bitset_empty();
		for (int iii = 0; iii < length; iii++ ){
			bitset_setBitValue(newPath, iii, bitset_memberOf(navPath,iii));
		}
		bitset_setBitValue(newPath, length, 1);
		traverseTree(binaryTree_rightChild(huffmanTree, pos), huffmanTree, 
			newPath, pathArray);
		//bitset_free(newPath);
	}
	
	/*If current position does not have a left or right child print the 
	  character*/
	if(!binaryTree_hasLeftChild(huffmanTree, pos) && 
		!binaryTree_hasRightChild(huffmanTree, pos)){
		freqChar* tmp = binaryTree_inspectLabel(huffmanTree, pos);
		pathArray[(int)tmp->character] = navPath;
		printf("%c : ", tmp->character);
		for(int iii = 0; iii < navPath->length; iii++){
		printf("%d", bitset_memberOf(navPath, iii ));
		}
		printf("\n");
	}
}

/*
 * encodeFile - function to encode input file
 *
 * Parameters:  inputfile     - file to be encoded
 *              outputfile    - file where encoded text is stored
 *              pathArray     - array with pointers to bitsets with binary code 
 *                              for all characters
 */
void encodeFile(FILE *encodeThis, FILE *output, bitset *pathArray[]){
	unsigned char tmp;
	int lengthCharBitset;
	int lengthCharCompound;
	bitset *compoundBitset = bitset_empty();
	char* writeToFile;
	int capacityCharCompound;


	while((tmp = fgetc(encodeThis))){
		if ( feof(encodeThis) ) {

			//adding the EOT char in the end of encoded bit sequence
			lengthCharBitset = pathArray[4]->length;
			for(int iii = 0; iii < lengthCharBitset; iii++){
				lengthCharCompound = compoundBitset->length;
				bitset_setBitValue(compoundBitset, lengthCharCompound, bitset_memberOf(pathArray[4], iii));
			}
			break;

		} else {
			lengthCharBitset = pathArray[(int)tmp]->length;
			for(int iii = 0; iii < lengthCharBitset; iii++) {
				if (compoundBitset->length == -1){
					lengthCharCompound = 0;
				} else {
					lengthCharCompound = compoundBitset->length;
				}
				bitset_setBitValue(compoundBitset,lengthCharCompound, 
					bitset_memberOf(pathArray[(int)tmp], iii));
				
				
			}
		}
	}

	writeToFile = toByteArray(compoundBitset);
	capacityCharCompound = compoundBitset->capacity;
	lengthCharCompound = compoundBitset->length;
	printf("%d TEST\n", (unsigned char)writeToFile[1]);

	printf("%d, %d\n", capacityCharCompound, lengthCharCompound);

    for(int iii = 0; iii < capacityCharCompound/8; iii++){
    //for(int iii = 0; iii < 10; iii++){
        //printf("%d ", (unsigned char)writeToFile[iii]);
        fputc((unsigned char)writeToFile[iii], output);
    }
    for(int iii = 0; iii < 80; iii++){
        printf("%d", bitset_memberOf(compoundBitset, iii ));
    }
    //printf("\n%d", sizeof(writeToFile));

	//fwrite(&writeToFile, 1, capacityCharCompound/8, output);

    //printf("length of bitset %d\n", compoundBitset->length);

}

/*
 * decodeFile - function to decode
 *
 * Parameters:  inputfile     - file to be decoded
 *              outputfile    - file where decoded text is stored
 *              huffmanTree   - tree used to decode
 * 
 * This function follows the path through the tree from the input file until a 
 * leaf is found, the character on this leaf is printed in the output file.
 */
void decodeFile(FILE* decodeThis, FILE* output, binary_tree* huffmanTree){
	//int finished = 0;
	//int failed = 0;
	int size;
	freqChar* tmp2;
	binaryTree_pos treePos = binaryTree_root(huffmanTree);
	fseek(decodeThis, 0, SEEK_END);
	size = ftell(decodeThis);
	fseek(decodeThis, 0, SEEK_SET);
	int tmp[size];
    bool decodeSequence[size*8];
	printf("%d\n", size);

	//size_t len = fread(tmp, sizeof(int8_t), 10, decodeThis);

	for (int iii = 0; iii < size; iii++) {
		tmp[iii] = fgetc(decodeThis);
        //printf("%c", (unsigned char)tmp[iii]);

	}
	//tmp[0] = 39;
    for (int iii = 0; iii < size; iii++) {
        //tmp[iii] = fgetc(decodeThis);
        //printf("%d ", tmp[iii]);

    }
	printf("\n");


    bool *toConvert;
    toConvert = (bool*) calloc(size*8,sizeof(bool));

    for (int iii = 0; iii < size; iii++){
        //int testo = tmp[iii];

        for (int jjj = 0; jjj < 8; jjj++) {
            toConvert[iii*8+jjj] = (tmp[iii] >> jjj) & 1;
        }
    }
	/*for (int iii = 0; iii < size*8; iii++){
		printf("%d", toConvert[iii]);
		if((iii+1)%8==0&&iii!=0){
			printf("\n");
		}
	}*/
    //treePos = binaryTree_root(huffmanTree); //DONE ALREADY
	printf("\n");
    for (int iii = 0; iii < size*8; iii++){
		//printf("%d\n", iii);
        if(binaryTree_hasLeftChild(huffmanTree, treePos)||binaryTree_hasRightChild(huffmanTree, treePos)){
            if(toConvert[iii] == 0){
                treePos = binaryTree_leftChild(huffmanTree, treePos);
            }
            if(toConvert[iii] == 1){
                treePos = binaryTree_rightChild(huffmanTree, treePos);
            }
        } else {
            tmp2 = (freqChar *) binaryTree_inspectLabel(huffmanTree, treePos);
            //printf("%c\n", tmp2->character);
            fprintf(output, "%c", tmp2->character);
            treePos = binaryTree_root(huffmanTree);
			iii--; //THIS IS THE FIX
            if ((int) tmp2 == 4) //WHAT DOES THIS DO?
                return;
			
        }

    }
	//treePos = binaryTree_root(huffmanTree);
	//int i=0;
	/*while(i<size*8){
		printf("%d\n", i);
		if(binaryTree_hasLeftChild(huffmanTree, treePos)||binaryTree_hasRightChild(huffmanTree, treePos)){
            if(toConvert[i] == 0){
                treePos = binaryTree_leftChild(huffmanTree, treePos);
				i++;
            }
            if(toConvert[i] == 1){
                treePos = binaryTree_rightChild(huffmanTree, treePos);
				i++;
            }
        } else {
            tmp2 = (freqChar *) binaryTree_inspectLabel(huffmanTree, treePos);
            printf("%c\n", tmp2->character);
            fprintf(output, "%c", tmp2->character);
            treePos = binaryTree_root(huffmanTree);
            if ((int) tmp2 == 4){ //WHAT DOES THIS DO?
                return;
			}
			i++;
        }
	}*/
    
	printf("File decoded successfully!\n");

}

/*
 * wrongArgs - function to print error message
 *
 * This function prints error message and usage and then returns 0.
 */
int wrongArgs(void){
	fprintf(stderr, "USAGE:\nhuffman [OPTION] [FILE0] [FILE1] [FILE2]\n");
	fprintf(stderr, "Options:\n-encode encodes FILE1 acording to the frequence" 
	" analysis done on FILE0. ");
	fprintf(stderr, "Stores the result in FILE2\n");
	fprintf(stderr, "-decode decodes FILE1 acording to the frequence analysis" 
	" done on FILE0. ");
	fprintf(stderr, "Stores the result in FILE2\n");
	return 0;
}