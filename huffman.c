/* An implementation of 'Huffman' coding, an algorithm for
 * data compression (Huffman 1952).
 *
 * The program is implemented as command line application
 *
 * Parameter:	- [-encode]/[-decode]
 * 				- [FILE1] text file that will be used to build
 * 				  the frequency table
 * 				- [FILE2] input that will be encoded/decoded
 * 				  depending on the first argument
 * 				- [FILE3] file name of output file that will
 * 				  be either huffman encoded or plain text.
 *
 * 	Output:     - the program returns 0 upon completion
 *
 * 	Comments:   The program uses the datatypes 'tree_3cell',
 * 				'list_2cell' and 'bitset' (all implemented
 * 				by Eliasson <johane@cs.umu.se> according
 * 				Janlert and Wiberg 2000), and 'prioqueue'
 * 				(implemented by Kallin-Westin <kallin@cs.umu.se>).
 *
 * Written by Simon Andersson <dv15san@cs.umu.se>
 * and Lorenz Gerber <dv15lgr@cs.umu.se>
 * February 18, 2016.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_3cell.h"
#include "prioqueue.h"
#include "bitset.h"


/*
 * Struct 'freqChar'
 * Node for 'tree_3cell' datatype that allows
 * to store both a frequency and a character value
 */
typedef struct {
  int value;
  unsigned char character;
} freqChar;

#define MAXBITSIZE 30

void getFrequency(int *frequency, FILE* file);
int compareTrees(VALUE tree1, VALUE tree2);
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE));
void traverseTree(binaryTree_pos pos,
                  binary_tree *huffmanTree, int navPath[],
                  int freeIndex, bitset *pathArray[]);
void encodeFile(FILE* encodeThis, FILE* output, bitset *pathArray[]);
void decodeFile(FILE* decodeThis, FILE* output, binary_tree* huffmanTree);
int wrongArgs(void);

int main(int argc, char **argv){


    /*
     * Variables
     */
    int frequency[256] = { 0 };
	int navPath[MAXBITSIZE] = {-1};
	int freeIndex = 0;


	/*
	 * Check number of command line arguments
	 */
    if(argc <= 4){
		return wrongArgs();
	}


	/*
	 * Check and switch for encode / decoden argument
	 */
    int selector = 0;
	char encodeStr[8];
	char decodeStr[8];

	strcpy(encodeStr, "-encode");
	strcpy(decodeStr, "-decode");
	if (!strcmp(argv[1], encodeStr)){
		selector = 1;
	} else if(!strcmp(argv[1], decodeStr)){
		selector = 2;
	} else return wrongArgs();



	/*
	 * Check and open files
	 */
    FILE* freqFilep = fopen(argv[2], "rt");
	if(freqFilep == NULL){
		fprintf(stderr, "Couldn't open frequency file %s\n", argv[2]);
		return wrongArgs();
	}

	FILE *infilep;
	if (selector == 1) {
		infilep = fopen(argv[3], "rt");
	} else {
		infilep = fopen(argv[3], "rb");
	}

	if(infilep == NULL){
		fprintf(stderr, "Couldn't open input file %s\n", argv[3]);
		return wrongArgs();
	}

	FILE *outfilep = fopen(argv[4], "w");

	if(outfilep == NULL){
		fprintf(stderr, "Couldn't open output file %s\n", argv[4]);
		return wrongArgs();
	}


    /*
     * Switch for two main modes:
     * encode / decode
     */
    switch(selector) {
		case 1:
			// Make frequency table
			getFrequency(frequency, freqFilep);
			
			// Build huffman tree
			binary_tree *treeEncode = buildHuffmanTree(frequency, compareTrees);
			
			// Traverse the tree to build a code table
			bitset *pathArray_encode[256];
			traverseTree(binaryTree_root(treeEncode), treeEncode, navPath,
                         freeIndex, pathArray_encode);
			
			// Encode the input file
			encodeFile(infilep, outfilep, pathArray_encode);
			
			// Free allocated memory
			for (int i = 0; i<256; i++){
				bitset_free(pathArray_encode[i]);
			}
			binaryTree_free(treeEncode);

			// Screen output
            long readBytes = ftell(infilep) + 1;
            long writeBytes = ftell(outfilep) + 1;
			printf("%ld bytes read from %s.\n", readBytes, argv[3]);
			printf("%ld bytes used in encoded form.", writeBytes);
			break;
			
		case 2:

			// Make frequency table
			getFrequency(frequency, freqFilep);
			
			// Build huffman tree
			binary_tree *treeDecode = buildHuffmanTree(frequency, compareTrees);

			// Decode the input file
			decodeFile(infilep, outfilep, treeDecode);

			binaryTree_free(treeDecode);
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
	
	// Increase freqeuncy of EOT character by 1
	frequency[4]++;
	
	while (finished!=1){
		ch = fgetc(file);

        // Stop reading at EOF
		if (ch == EOF){
			finished=1;
		} else {

			// increase the frequency of the character read
			frequency[ch]++;
		}
	}
	
	/*
	 * The following lines solve the problem high high numbers of zero frequency
	 * characters. They lead to unbalanced and unnecessary deep trees.
	 * We first multiply the frequency by 1000 to introduce a large value gap
	 * between characters with zero frequency and such with frequency > 0.
	 * Then we set all characters with frequency 0 to frequency 1. This will
	 * result in two almost independent sub-trees.
	 */
	for(int iii = 0; iii < 256; iii++){
		frequency[iii] *= 1000;
		if(frequency[iii]==0){
			frequency[iii]=1;
		}
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
	
	// Compares the frequency value in the struct
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
    int allChars;
	binary_tree *tree1;
	binary_tree *tree2;
	binary_tree *newTree; 
	
	/*
	 * Create one tree for each character and put all of them in a
	 * priority queue.
	 */
	for (allChars = 0; allChars < 256; allChars++){
		freqChar *nodeLabel = malloc(sizeof(freqChar));
		nodeLabel->character = allChars;
		nodeLabel->value = frequency[allChars];
		newTree = binaryTree_create(); 
		binaryTree_setMemHandler(newTree, free);
		binaryTree_setLabel(newTree, nodeLabel, binaryTree_root(newTree));
		pqueue_insert(treebuildingQueue, newTree);
	}
	

    /*
	 * While priority queue isn't empty take out the two front values and
	 * connect these two trees with a new node (tree), put this new combined
	 * tree in the queue.
	 */
    while(!pqueue_isEmpty(treebuildingQueue)){

        // Create new tree with one node
		newTree = binaryTree_create(); 
		binaryTree_setMemHandler(newTree, free);


		/*
		 * Take out the first tree from the queue and save the values on its
		 * label.
		 */
		tree1 = pqueue_inspect_first(treebuildingQueue);
		freqChar *labelTree1 =
                binaryTree_inspectLabel(tree1, binaryTree_root(tree1));
		pqueue_delete_first(treebuildingQueue);
		
		// When the last tree has been taken out return that tree
		if(pqueue_isEmpty(treebuildingQueue)){
			binaryTree_free(newTree);
			binaryTree_setMemHandler(tree1, free);
			pqueue_free(treebuildingQueue); 
			return tree1; 
		}
		else{
			
			/*
			 * Take out the second tree from the queue and save the values on
			 * its label.
			 */
			tree2 = pqueue_inspect_first(treebuildingQueue);
			freqChar *labelTree2 =
                    binaryTree_inspectLabel(tree2, binaryTree_root(tree2));
			pqueue_delete_first(treebuildingQueue);
			
			// Initiate and give values to the new node label
			freqChar *labelCombinedTree = malloc(sizeof(freqChar));
			labelCombinedTree->value = labelTree1->value + labelTree2->value;
			labelCombinedTree->character = -1;
			
			// Set label on the new tree and insert a left and right child
			binaryTree_setLabel(newTree, labelCombinedTree,
				binaryTree_root(newTree));
			
			/*
			 * Set the two trees from the queue
			 * as right/left child on the
			 * new node.
			 */
			newTree->root->rightChild = tree1->root;
			newTree->root->leftChild = tree2->root;
			tree1->root->parent = newTree->root;
			tree2->root->parent = newTree->root;
			
			// Insert the new tree in the queue
			pqueue_insert(treebuildingQueue, newTree);
			free(tree1);
			free(tree2);
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
void traverseTree(binaryTree_pos pos, binary_tree *huffmanTree, int navPath[],
                  int freeIndex, bitset *pathArray[]){
	// If we move left in the tree add '0' to the bit sequence
	if(binaryTree_hasLeftChild(huffmanTree, pos)){
		navPath[freeIndex] = 0;
		traverseTree(binaryTree_leftChild(huffmanTree, pos), huffmanTree, 
			navPath, freeIndex+1, pathArray);
	}
	
	// If we move right in the tree add '1' to the bit sequence
	if(binaryTree_hasRightChild(huffmanTree, pos)){
		navPath[freeIndex] = 1;
		traverseTree(binaryTree_rightChild(huffmanTree, pos), huffmanTree, 
			navPath, freeIndex+1, pathArray);
	}
	
	/*
	 * If current position does not have a left or right child save the bit
	 * sequence in a bitset and store the adress to this bitset.
	 */
	if(!binaryTree_hasLeftChild(huffmanTree, pos) && 
		!binaryTree_hasRightChild(huffmanTree, pos)){
		freqChar* labelLeaf = binaryTree_inspectLabel(huffmanTree, pos);
		bitset* result = bitset_empty();
		int i;
		for(i=0; i<freeIndex; i++){
			bitset_setBitValue(result, i, navPath[i]);
		}
		pathArray[(int)labelLeaf->character] = result;
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
		if (feof(encodeThis)) {
			// Adds the EOT character at the end of the encoded bit sequence
			lengthCharBitset = pathArray[4]->length;
			for(int iii = 0; iii < lengthCharBitset; iii++){
				lengthCharCompound = compoundBitset->length;
				bitset_setBitValue(compoundBitset, lengthCharCompound,
                                   bitset_memberOf(pathArray[4], iii));
			}
			break;
		} else {
			// Take all bit sequences and add them all to one bitset
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
	
	// Write the whole bitset to output file
	writeToFile = toByteArray(compoundBitset);
	capacityCharCompound = compoundBitset->capacity;
	for(int iii = 0; iii < capacityCharCompound/8; iii++){
		fputc((unsigned char)writeToFile[iii], output);
	}
	
	// Free allocated memory
	free(writeToFile);
	bitset_free(compoundBitset);
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
	int size;
	int decodingPos = 0;
	freqChar* currentLabel;
	binaryTree_pos treePos = binaryTree_root(huffmanTree);
	
	// Get length of input file
	fseek(decodeThis, 0, SEEK_END);
	size = ftell(decodeThis);
	fseek(decodeThis, 0, SEEK_SET);
	int inputTextArray[size];
	
	// Read all characters
	for (int currentRead = 0; currentRead < size; currentRead++) {
		inputTextArray[currentRead] = fgetc(decodeThis);
	}

    // Characters have to be translated into bitsequence
	bool *bitSeqToConvert;
	bitSeqToConvert = (bool*) calloc(size * 8,sizeof(bool));
	
	// Convert every read character to a binary sequence
	for (int currentTranslateChar = 0; currentTranslateChar < size;
         currentTranslateChar++){
		for (int bytePosition = 0; bytePosition < 8; bytePosition++) {
			bitSeqToConvert[currentTranslateChar * 8 + bytePosition] =
                    (inputTextArray[currentTranslateChar] >> bytePosition) & 1;
		}
	}
	
	/*
	 * According to binary sequence move through tree until leaf is reached at
	 * leaf print associated character to output file.
	 */
	while(decodingPos < size * 8){
		if(binaryTree_hasLeftChild(huffmanTree, treePos) ||
                binaryTree_hasRightChild(huffmanTree, treePos)){
			if(bitSeqToConvert[decodingPos] == 0){
				treePos = binaryTree_leftChild(huffmanTree, treePos);
				decodingPos++;
			}
			else if(bitSeqToConvert[decodingPos] == 1){
				treePos = binaryTree_rightChild(huffmanTree, treePos);
				decodingPos++;
			}
		}
		else{
			currentLabel = (freqChar *) binaryTree_inspectLabel(huffmanTree, treePos);
			fprintf(output, "%c", currentLabel->character);
			treePos = binaryTree_root(huffmanTree);
		}
	}
	free(bitSeqToConvert);
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