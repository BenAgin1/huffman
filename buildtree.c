#include <stdio.h>
#include "tree_3cell.h"
#include "prioqueue.h"
#include "bitset.h"
#include <stdlib.h>
#include <string.h>

/* Node of the huffman tree */
typedef struct {
  int value;
  char character;
} freqChar;

typedef struct node Node;

void getFrequency(int *frequency, FILE* file);
int compareTrees(VALUE tree1, VALUE tree2);
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE));
void traverseTree(binaryTree_pos pos, binary_tree* huffmanTree);

int main(int argc, char **argv){
	int frequency[256];
	int character;
	if(argc <= 2){
		fprintf(stderr, "Usage: input file, output file\n");
		return 0;
	}
	
	FILE* infilep = fopen(argv[1], "rt");
	if(infilep == NULL){
		fprintf(stderr, "Couldn't open input file %s\n", argv[1]);
	}
	
	FILE* outfilep = fopen(argv[2], "w");
	if(outfilep == NULL){
		fprintf(stderr, "Couldn't open output file %s\n", argv[2]);
	}
	
	getFrequency(frequency, infilep);
	printf("HEJEJEJEJ\n");
	binary_tree *tree4 = buildHuffmanTree(frequency, compareTrees);
	/*for (character = 0; character < 256; character++){
		printf("%c %d : %d\n", (char)character, character, frequency[character]);
	}*/
	pqueue *treebuildingQueue = pqueue_empty(compareTrees);
	pqueue_setMemHandler(treebuildingQueue, free);
	for (character = 0; character < 256; character++){
		freqChar *tmp=malloc(sizeof(freqChar));
		tmp->character = character;
		tmp->value = frequency[character];
		binary_tree* test = binaryTree_create();
		binaryTree_setLabel(test, tmp, binaryTree_root(test));
		pqueue_insert(treebuildingQueue, test);
	}
	////////////////////////////////////////////////
	//QUEUE TESTING ETC
	//TESTED QUEUE SEEMS TO WORK AS INTENDED
	/*binary_tree *lekis = pqueue_inspect_first(treebuildingQueue);
	freqChar *lekis2 = binaryTree_inspectLabel(lekis, binaryTree_root(lekis));
	printf("%d\n", lekis2->value);
	
	pqueue_delete_first(treebuildingQueue);
	lekis = pqueue_inspect_first(treebuildingQueue);
	lekis2 = binaryTree_inspectLabel(lekis, binaryTree_root(lekis));
	printf("%d\n", lekis2->value);
	
	pqueue_delete_first(treebuildingQueue);
	lekis = pqueue_inspect_first(treebuildingQueue);
	lekis2 = binaryTree_inspectLabel(lekis, binaryTree_root(lekis));
	printf("%d\n", lekis2->value);*/
	/////////////////////////////////////////////////
	//OLD CODE OUTSIDE OF FUNCTION
	binary_tree *tree3;
	int i=0;
	while(!pqueue_isEmpty(treebuildingQueue)){
		binary_tree *newTree = binaryTree_create();
		binary_tree *tree1 = pqueue_inspect_first(treebuildingQueue);
		tree3=tree1;
		freqChar *leka2 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
		pqueue_delete_first(treebuildingQueue);
		freqChar *leka = malloc(sizeof(freqChar));
		if(pqueue_isEmpty(treebuildingQueue)){
			break;
		}
		else{
			binary_tree *tree2 = pqueue_inspect_first(treebuildingQueue);
			freqChar *leka3 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
			pqueue_delete_first(treebuildingQueue);
			
			//freqChar *leka2 = binaryTree_inspectLabel(tree1, binaryTree_root(tree1));
			//freqChar *leka3 = binaryTree_inspectLabel(tree2, binaryTree_root(tree2));
			//freqChar *leka4 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
			//printf("%d\n", leka2->value+i);
			leka->value = leka2->value + leka3->value;
			leka->character = -1;
			//printf("%d\n", i);
			binaryTree_setLabel(newTree, leka, binaryTree_root(newTree));
			binaryTree_insertLeft(newTree, binaryTree_root(newTree));
			binaryTree_insertRight(newTree, binaryTree_root(newTree));
			newTree->root->rightChild = tree1->root;
			newTree->root->leftChild = tree2->root;
			
			pqueue_insert(treebuildingQueue, newTree);
		}
		i++;
	}
	//////////////////////////////////////////////////////////////////////
	//SOME TESTS FOR THE COMPLETE TREE
	traverseTree(binaryTree_root(tree4), tree4);
	binaryTree_pos hejsan = binaryTree_root(tree4);
	int j=0;
	while(binaryTree_hasLeftChild(tree4, hejsan)){
			hejsan = binaryTree_leftChild(tree4, hejsan);
			j++;
	}
	freqChar *lekstuga = binaryTree_inspectLabel(tree4, binaryTree_rightChild(tree4, binaryTree_leftChild(tree4, binaryTree_root(tree4))));
	printf("%d\n", lekstuga->value);
	freqChar *lekstuga2 = binaryTree_inspectLabel(tree4, hejsan);
	freqChar *lekstuga3 = binaryTree_inspectLabel(tree4, binaryTree_root(tree4));
	printf("%d\n", lekstuga3->value);
	printf("%d\n", lekstuga2->value);
	printf("%c\n", lekstuga2->character);
	printf("Antal steg: %d\n", j);
	
	printf("%d\n", frequency[0]);
	fclose(infilep);
	fclose(outfilep);
	pqueue_free(treebuildingQueue);
	return 0;
}

void getFrequency(int* frequency, FILE* file){
	int finished=0;
	int ch;
	for (ch = 0; ch < 256; ch++){
		frequency[ch] = 0;
	}
	
	while (finished!=1){
		ch = fgetc(file);
		//printf("%d ", ch);
		/* end of file or read error.  EOF is typically -1 */
		if (ch == EOF){
			finished=1;
		}
		/* assuming ASCII; "letters" means "a to z" */
		else{
			frequency[ch]++;
		}
	}
}

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

binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE)){
	pqueue *treebuildingQueue = pqueue_empty (compare);
	//pqueue_setMemHandler(treebuildingQueue, free); // CANT USE THIS CAUSE THE FINISHED TREE WILL BE REMOVED
	//PERHAPS THE PRIO QUEUE SHOULD BE BUILT IN A DIFFERENT FUNCTION?
	int chartmp;
	binary_tree *huffmanTree = binaryTree_create();
	
	/* Create 1 tree for each character and put all of them in a priority queue*/
	for (chartmp = 0; chartmp < 256; chartmp++){
		freqChar *tmp=malloc(sizeof(freqChar));
		tmp->character = chartmp;
		tmp->value = frequency[chartmp];
		binary_tree* treetmp = binaryTree_create();
		binaryTree_setLabel(treetmp, tmp, binaryTree_root(treetmp));
		pqueue_insert(treebuildingQueue, treetmp);
	}
	
	/*While priority queue isn't empty take out the two front values and 
	connect these two trees with a new node (tree), put this new combined tree 
	in the queue.*/
	while(!pqueue_isEmpty(treebuildingQueue)){
		
		/*Create new tree with 1 node.*/
		binary_tree *newTree = binaryTree_create();
		
		/*Take out the first tree from the queue and save the values on its label.*/
		binary_tree *tree1 = pqueue_inspect_first(treebuildingQueue);
		freqChar *freqChartmp1 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
		pqueue_delete_first(treebuildingQueue);
		
		/*Initiate struct to put on the new node's label.*/
		freqChar *freqChartmp2 = malloc(sizeof(freqChar));
		
		/*When the last tree has been taken out return that tree*/
		if(pqueue_isEmpty(treebuildingQueue)){
			huffmanTree = tree1;
			//pqueue_free(treebuildingQueue); !!!REMOVES THE TREE!!!
			return huffmanTree;
		}
		else{
			/*Take out the second tree from the queue and save the values on its label.*/
			binary_tree *tree2 = pqueue_inspect_first(treebuildingQueue);
			freqChar *freqChartmp3 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
			pqueue_delete_first(treebuildingQueue);
			
			/*Give values to the label.*/
			freqChartmp2->value = freqChartmp1->value + freqChartmp3->value;
			freqChartmp2->character = -1;
			
			/*Set label on the new tree and insert a left and right child.*/
			binaryTree_setLabel(newTree, freqChartmp2, binaryTree_root(newTree));
			binaryTree_insertLeft(newTree, binaryTree_root(newTree));
			binaryTree_insertRight(newTree, binaryTree_root(newTree));
			
			/*Set the two trees from the queue as right/left child on the new node.*/
			newTree->root->rightChild = tree1->root;
			newTree->root->leftChild = tree2->root;
			
			/*Insert the new tree in the queue.*/
			pqueue_insert(treebuildingQueue, newTree);
		}
	}
	pqueue_free(treebuildingQueue);
	return 0;
}

void traverseTree(binaryTree_pos pos, binary_tree *huffmanTree){
	
	if(binaryTree_hasLeftChild(huffmanTree, pos)){
		traverseTree(binaryTree_leftChild(huffmanTree, pos), huffmanTree);
	}
	if(binaryTree_hasRightChild(huffmanTree, pos)){
		traverseTree(binaryTree_rightChild(huffmanTree, pos), huffmanTree);
	}
	
	/*If current position does not have a left or right child print the character*/
	if(!binaryTree_hasLeftChild(huffmanTree, pos)&&!binaryTree_hasRightChild(huffmanTree, pos)){
		freqChar* tmp = binaryTree_inspectLabel(huffmanTree, pos);
		printf("%c : %d\n", tmp->character, tmp->value);
	}
	
}