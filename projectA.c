#include <assert.h>
#include <memory.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

#include "projectA.h"

#define DEBUG

// an array of blocks, body of the buffer pool. 
block * bufferPool = NULL; 

// a direct-address table to store fd-metadata pairs.
metadata * fdMetaTable = NULL;

// global variable used for buffer pool replacement policy. 
// From 1 to BUFFERSIZE.
int current = 1;

// the max number of file descriptor that the system 
// can assign for this process.
long long max_fd = -1;

void init_block(block* blockPtr)
{
	blockPtr->pinCount = 0;
	blockPtr->dirty = 0;
	blockPtr->fd =  -1;
	blockPtr->blockID = INT_MAX;
	blockPtr->referenced = 0;
	blockPtr->freeSpace = -1;
	blockPtr->pageLocation = NULL;
	blockPtr->data = NULL;

	#ifdef DEBUG
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

	// get the maximum number of fd, and initialize fdMetaTable
	// (direct-address table of fd and metadata pairs);
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
		exit(1);
	}
	max_fd = rlim.rlim_cur - 3; //because fd = 0, 1, 2 don't count.
	fdMetaTable = (metadata *)malloc(max_fd * sizeof(metadata));

	// create a folder called data to store data of the database system.
	struct stat st;
	if (stat("./data", &st) == -1) {
	    mkdir("./data", 0700);
	}
	#ifdef DEBUG
		printf("********* BM_init ***********\n");
	#endif
}

// helper function, generate full path of a file from filename and ID.
void get_location(char *str, const char *filename, const int blockID) {
	char ID[(int)log10(INT_MAX) + 1]; 
	sprintf(ID, "%d", blockID);
	strcpy(str, "./data/");
	strcat(str, filename);
	strcat(str, "_");
	strcat(str, ID);
	strcat(str, ".dat");
}

//helper function, get location of metadata page.
void get_meta_location(char *dest, const char *filename) {
	strcpy(dest, "./data/");
	strcat(dest, filename);
	strcat(dest, "_meta.dat");
}

// Create a new header page for the file, and create a metadata page.
errCode BM_create_file( const char *filename ) {
	char str[LOCATIONSIZE];
	get_location(str, filename, -1);

	#ifdef DEBUG
		printf("str= %s\n", str);
	#endif
	
	FILE *fp = fopen(str, "wb+");
	if (fp == NULL) { return 1; }
	// set the page size to 4096bytes, and ends with a '@'.
	fseek(fp, 4095, 0);
	fputc('@', fp);
	fclose(fp);

	// create metadata page.
	char metaLocation[LOCATIONSIZE];
	get_meta_location(metaLocation, filename);
	fp = fopen(metaLocation, "wb+");
	if (fp == NULL) { return 1; }
	int meta[2] = {0, 1};
	fwrite(meta, sizeof(int), 2, fp);
	fclose(fp);

	#ifdef DEBUG
		printf("********* BM_create_file ***********\n");
	#endif
	return 0;
}

// helper function, add a block into buffer pool, implement replacement
// policy here. Use clock replacement policy. Store the address of block
// in *block_2ptr.
errCode buffer_add_block(block **block_2ptr, const void *data, fileDesc fd,\
		int blockID, const char *pageLocation) {

	int start = current;
	int m = 0;
	while (m < 2) {

		#ifdef DEBUG
			printf("pos = %d\n", current - 1);
		#endif

		if (bufferPool[current - 1].pinCount == 0) {
			if (bufferPool[current - 1].referenced == 0) {
				// replace with this block, if dirty == 1, write back.
				// need to release memory of data and pageLocation first.
				if (bufferPool[current - 1].dirty == 1) {
					FILE *fp = fopen(bufferPool[current - 1].pageLocation, "wb+");
					if (fwrite(bufferPool[current - 1].data, 1, 4096, fp) != 4096) { 
						return 2; 
					}
				}
				free(bufferPool[current - 1].data);
				free(bufferPool[current - 1].pageLocation);
				init_block(&bufferPool[current - 1]);
				bufferPool[current - 1].pinCount = 1;
				bufferPool[current - 1].blockID = blockID;
				bufferPool[current - 1].fd = fd;
				bufferPool[current - 1].referenced = 1;
				bufferPool[current - 1].data = (char *)malloc(4096);
				memcpy(bufferPool[current - 1].data, data, 4096);
				bufferPool[current - 1].pageLocation = (char *)malloc(LOCATIONSIZE);
				strcpy(bufferPool[current - 1].pageLocation, pageLocation);
				*block_2ptr = &bufferPool[current - 1];

				#ifdef DEBUG
					printf("********* buffer_add_block *********\n");
				#endif

				return 0;
			}
			bufferPool[current - 1].referenced = 0;
		}
		current = (current == BUFFERSIZE) ? 1 : current + 1;
		if (current == start) { m++; }
	}
	return 2;
}

// open the first header page of the file, get fd, open metadata page, store
// metadata.
fileDesc BM_open_file( const char *filename ) {
	char fileLocation[LOCATIONSIZE];
	get_location(fileLocation, filename, -1);
	FILE *fp = fopen(fileLocation, "rb+");
	if (fp == NULL) { return -1; }
	fileDesc fd = fileno(fp);
	
	//read metadata from metadata page.
	char metaLocation[LOCATIONSIZE];
	get_meta_location(metaLocation, filename);
	FILE *pMeta = fopen(metaLocation, "rb");
	if (pMeta == NULL) { return -1; }
	int meta[2];
	fread(meta, sizeof(int), 2, pMeta);
	
	// store metadata in direct-address table.
	metadata temp;
	temp.fileName = (char *)malloc(LOCATIONSIZE);
	strcpy(temp.fileName, filename);
	temp.blockNumber = meta[0];
	temp.headerNumber = meta[1];
	fdMetaTable[fd - 3] = temp;

	#ifdef DEBUG
		printf("fd = %d\n", fd);
		printf("blockNumber: %d\theaderNumber: %d\n", meta[0], meta[1]);
		printf("open file: %s\n", fileLocation);
		printf("********** BM_open_file ***********\n");
	#endif

	return fd;
}


errCode BM_close_file( fileDesc fd ) {
	// scan buffer pool, see if there are still blocks of this file pinned
	for (int i = 0; i < BUFFERSIZE; ++i) {
		if (bufferPool[i].fd == fd) {
			if (bufferPool[i].pinCount != 0) {
				return 3;
			}
		}
	}
	// write back metadata to metadata page.
	metadata temp = fdMetaTable[fd - 3];
	char metaLocation[LOCATIONSIZE];
	get_meta_location(metaLocation, temp.fileName);
	FILE *fp = fopen(metaLocation, "wb+");
	if (fp == NULL) { return 3; }
	int meta[2] = {temp.blockNumber, temp.headerNumber};
	fwrite(meta, sizeof(int), 2, fp);
	fclose(fp);

	//delete fd and metadata from hash table.
	free(fdMetaTable[fd - 3].fileName);
	fdMetaTable[fd - 3].fileName = NULL;
	fdMetaTable[fd - 3].blockNumber = 0;
	fdMetaTable[fd - 3].headerNumber = 0;

	//close file using fd.
	if (close(fd) != 0) { return 3; }
	return 0;

	#ifdef DEBUG
		printf("meta page location: %s\n", metaLocation);
		printf("********** BM_close_file **********\n");
	#endif
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
