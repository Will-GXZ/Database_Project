#include <assert.h>
#include <memory.h>
#include "projectA.h"


//populates an existing block with some random data
void populate_block(block* blockPtr);


int main(int argc, char const *argv[])
{
	int errc = 0;
	/*
	bm init
	*/
	BM_init();

	/*
	create a file 
	insert pages filled with data
	close a file
	*/
	BM_create_file("testing.dat");
	fileDesc fd = BM_open_file("testing.dat");
	BM_alloc_block(fd);
	block* blockPtr=NULL;
	
	BM_get_first_block(fd, &blockPtr); // this function will assign blockPtr to a valid block from the BP
	populate_block(blockPtr);
	blockPtr->dirty=1; //we just made changes
	
	//make sure the loaded frame can actually be swapped out (when the corresponding file is closed).
	//we do not expect to close a file with pinned blocks
	BM_unpin_block(blockPtr);	
	assert(blockPtr->pinCount==0); //pin count should be 0	
	BM_close_file(fd);

	/*
	bm open file
	loop this stuff
	bm alloc block
	bm get first block 
	insert some data into block
	bm unpin block
	end loop
	bm close file
	*/
	fd = BM_open_file("testing.dat");
	int i;
	for (i = 0; i < 99; i++) {
		BM_alloc_block(fd);
	}
	BM_get_first_block(fd, &blockPtr);
	populate_block(blockPtr);
	blockPtr->dirty=1; //we just made changes
	BM_unpin_block(blockPtr);
	assert(blockPtr->pinCount==0); //pin count should be 0	
	for (i = 0; i < 99; i++) {
		BM_get_next_block(fd, &blockPtr);
		populate_block(blockPtr);
		blockPtr->dirty=1; //we just made changes
		BM_unpin_block(blockPtr);
		assert(blockPtr->pinCount==0); //pin count should be 0	
	}

	BM_get_first_block(fd, &blockPtr);
	
	//we do not expect to close a file with pinned blocks
	BM_unpin_block(blockPtr);
	assert(blockPtr->pinCount==0); //pin count should be 0	
	for (i = 0; i < 99; i++) {
		BM_get_next_block(fd, &blockPtr);
		//we do not expect to close a file with pinned blocks
		BM_unpin_block(blockPtr);
		assert(blockPtr->pinCount==0); //pin count should be 0	
	}

	BM_close_file(fd);
	// file should have 100 blocks now


	/*
	open the above file 
	access pages
	close the file
	*/
	char testing_elements[50];
	fd = BM_open_file("testing.dat");
	for (i = 0; i < 50; i++) {
		BM_get_this_block(fd, (i * i + i) % 100, &blockPtr);
		populate_block(blockPtr);
		testing_elements[i]=blockPtr->data[i];
		//we do not expect to close a file with pinned blocks
		blockPtr->dirty=1;
		BM_unpin_block(blockPtr);
		assert(blockPtr->pinCount==0); //pin count should be 0	
	}
	BM_close_file(fd);


	/*
	bm open speficic file (checks that its opening an existing file not making a new one)
	bm get blocks and examine data
	bm close file
	*/
	
	fd = BM_open_file("testing.dat");
	for (i = 0; i < 50; i++) {
		BM_get_this_block(fd, (i * i + i) % 100, &blockPtr);
		assert(testing_elements[i]==blockPtr->data[i]);//assert we read the same bytes we wrote
		BM_unpin_block(blockPtr);
		assert(blockPtr->pinCount==0); //pin count should be 0	
	}
	BM_close_file(fd);


	/*
	open the above file 
	remove pages
	close the file
	*/
	fd = BM_open_file("testing.dat");
	for (i = 0; i < 100; i++) {
		BM_dispose_block(fd, i);
	}
	BM_close_file(fd);


	/*
	bm open speficic file (checks that its opening an existing file not making a new one)
	bm get first block
	bm dispose block
	bm close file
	*/

	fd = BM_open_file("testing.dat");
	BM_get_first_block(fd, &blockPtr);
	BM_unpin_block(blockPtr);
	assert(blockPtr->pinCount==0); //pin count should be 0	
	BM_dispose_block(fd, 0);
	BM_close_file(fd);

	
	/*
	all throughout print some errors if there are any.
	*/

    return 0;
}


void populate_block(block* blockPtr) {
	
	//assumes a block retrieved from BP, updates its contents
	assert(blockPtr!=NULL);

	char* raw_data = malloc(sizeof(char) * FRAMESIZE);
	int i;
	char tmp ;
	for (i = 0; i < FRAMESIZE; i++){
		tmp=i%78+'0'; 
		raw_data[i] = tmp;
	}
	//here we copy to the contents of the block
	memcpy(blockPtr->data, raw_data, FRAMESIZE);
	free(raw_data);
}


