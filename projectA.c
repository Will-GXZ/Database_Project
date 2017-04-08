#include <assert.h>
#include <memory.h>
#include "projectA.h"

// #define NDEBUG

// an array of blocks, body of the buffer pool. 
block* bufferPool; 

// a direct-address table to store fd-metadata pairs.
metadata* fdMetaTable;

// global variable used for buffer pool replacement policy. 
// From 1 to BUFFERSIZE.
int current;

void init_block(block* blockPtr)
{
	blockPtr->pinCount = 0;
	blockPtr->dirty = 0;
	blockPtr->fd =  -1;
	blockPtr->blockID = 0;
	blockPtr->referenced = 1;
	blockPtr->freeSpace = -1;
	blockPtr->pageLocation = NULL;
	blockPtr->data = NULL;

	#ifndef NDEBUG
		printf("************ init_block ************\n");
	#endif
}

void BM_init(){
	// create an array of blocks as buffer pool.
	bufferPool = (block *)malloc(BUFFERSIZE * sizeof(block));
	// initialize each block in the buffer poll.
	for(int i = 0; i < BUFFERSIZE; ++i) {
		init_block(&bufferPool[i]);
	}
	#ifndef NDEBUG
		printf("********* BM_init ***********\n");
	#endif
}

errCode BM_create_file( const char *filename ) {
	fprintf(stderr, "Calling BM_create_file with name: %s\n", filename);
	return 0;
}

fileDesc BM_open_file( const char *filename ) {
	fprintf(stderr, "Calling BM_open_file with name: %s\n", filename);
	return 0;
}

errCode BM_close_file( fileDesc fd ) {
	fprintf(stderr, "Attempting to close file: %d\n", fd);
	return 0;
}

errCode BM_get_first_block( fileDesc fd, block** blockPtr ) {
	fprintf(stderr, "Attempting to read first block from file %d\n", fd);

	return 0;
}


errCode BM_get_next_block( fileDesc fd, block** blockPtr ) {
	fprintf(stderr, "Attempting to get next block from file %d\n", fd);

	return 0;
}

errCode BM_get_this_block( fileDesc fd, int blockID, block** blockPtr ) {
	fprintf(stderr, "Attempting to get block %d from file %d\n", blockID, fd);

	return 0;
}

errCode BM_alloc_block( fileDesc fd ) {
	fprintf(stderr, "Attempting to allocate block in file %d\n", fd);
	return 0;
}

errCode BM_dispose_block( fileDesc fd, int blockID ) {
	fprintf(stderr, "Attempting to dispose of block %d from file %d\n",blockID, fd);
	return 0;
}

errCode BM_unpin_block( block* blockPtr ) {
	fprintf(stderr, "Unpinning block\n");

	return 0;
}

void BM_print_error( errCode ec ) {
	fprintf(stderr, "Printing error code %d\n", ec);
}
