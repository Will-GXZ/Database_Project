#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include "projectA.h"

/*
 * This is a dummy implementation to make sure that everything compiles.
 * You should expand this.
 * */

void init_block(block** bptr)
{
	*bptr = malloc(sizeof(block));
	(*bptr)->data = malloc(FRAMESIZE);
	(*bptr)->dirty=0;
	(*bptr)->pinCount=0;
}


void populate_block(block* blockPtr);


void BM_init(){
	fprintf(stderr, "Calling BM_init");
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
	block* new_pointer = NULL;
	init_block(&new_pointer);
	new_pointer->pinCount++;
	*blockPtr = new_pointer;
	return 0;
}


errCode BM_get_next_block( fileDesc fd, block** blockPtr ) {
	fprintf(stderr, "Attempting to get next block from file %d\n", fd);
	block* new_pointer = NULL;
	init_block(&new_pointer);
	new_pointer->pinCount++;
	*blockPtr = new_pointer;
	return 0;
}

errCode BM_get_this_block( fileDesc fd, int blockID, block** blockPtr ) {
	fprintf(stderr, "Attempting to get block %d from file %d\n", blockID, fd);
	block* new_pointer = NULL;
	init_block(&new_pointer);
	new_pointer->pinCount++;

	populate_block(new_pointer);
	*blockPtr = new_pointer;
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
	blockPtr->pinCount--;
	return 0;
}

void BM_print_error( errCode ec ) {
	fprintf(stderr, "Printing error code %d\n", ec);
}
