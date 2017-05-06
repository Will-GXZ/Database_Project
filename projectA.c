#include <assert.h>
#include <memory.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

#include "projectA.h"

// #define DEBUG

// an array of blocks, body of the buffer pool. 
block *bufferPool = NULL; 

// a direct-address table to store fd-metadata pairs.
metadata *fdMetaTable = NULL;

// global variable used for buffer pool replacement policy. 
// From 1 to BUFFERSIZE.
int current = 1;

// the max number of file descriptor that the system 
// can assign for this process.
long long max_fd = -1;

// the number of entries a page can contain
int pageCapacity = -1;

// the number of pageID's a header file can contain
int headerCapacity = -1;

void init_metadata(metadata* metaPtr) {
	metaPtr->firstBlockID = -1;
	metaPtr->lastBlockID = -1;
	metaPtr->blockNumber = -1;
	metaPtr->headerNumber = -1;
	metaPtr->fileName = NULL;
	metaPtr->currentID = -1;
	metaPtr->currentRecID = -1;
	metaPtr->fp = NULL;
}

void init_block(block* blockPtr) {
	blockPtr->pinCount = 0;
	blockPtr->dirty = 0;
	blockPtr->fd =  -1;
	blockPtr->blockID = INT_MAX;
	blockPtr->referenced = 0;
	blockPtr->freeSpace = -1;
	blockPtr->data = NULL;

	#ifdef DEBUG
		printf("************ init_block ************\n");
	#endif
}

void BM_init(){
	pageCapacity = (FRAMESIZE - 1 - sizeof(int)) / (ENTRYLENGTH + 1);
	headerCapacity = (FRAMESIZE - 1 - sizeof(int));

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
	for (int j = 0; j < max_fd; ++j) {
		init_metadata(&fdMetaTable[j]);
	}

	// create a folder called data to store data of the database system.
	struct stat st;
	if (stat("./data", &st) == -1) {
	    mkdir("./data", 0700);
	}
	#ifdef DEBUG
		printf("pageCapacity = %d\theaderCapacity = %d\n", pageCapacity, \
			headerCapacity);
		printf("********* BM_init ***********\n");
	#endif
}

// helper function, generate full path of a file from filename and ID.
void get_location(char *str, const char *filename, const char *postfix) {
	strcpy(str, "./data/");
	strcat(str, filename);
	strcat(str, "_");
	strcat(str, postfix);
}

// Create a new header page for the file, and create a metadata page.
errCode BM_create_file( const char *filename ) {
	char str[LOCATIONSIZE];
	get_location(str, filename, "h0.head");

	#ifdef DEBUG
		printf("headerLocation = %s\n", str);
	#endif
	
	FILE *fp = fopen(str, "wb+");
	if (fp == NULL) { return 1; }
	
	// set the header page size to FRAMESIZEbytes, and ends with a '@'.
	// Initialize the header page, fill directy with all -1's (not allocated).
	char data[FRAMESIZE] = {0};
	memset(data, -1, FRAMESIZE - 1 - sizeof(int));
	*((int *)(data + headerCapacity)) = headerCapacity;
	data[FRAMESIZE - 1] = '@';
	if (fwrite(data, 1, FRAMESIZE, fp) != FRAMESIZE) { return 1; } //write error
	fclose(fp);
	fp = NULL;

	// create metadata page.
	char metaLocation[LOCATIONSIZE];
	get_location(metaLocation, filename, ".meta");
	fp = fopen(metaLocation, "wb+");
	if (fp == NULL) { return 1; }
	int meta[4] = {-1, -1, 0, 1};
	fwrite(meta, sizeof(int), 4, fp);
	fclose(fp);

	#ifdef DEBUG
		printf("********* BM_create_file ***********\n");
	#endif
	return 0;
}

// helper function, add a block into buffer pool, implement replacement
// policy here. Use clock replacement policy. Store the address of block
// in *block_2ptr. Take an array of data, a file descriptor and a blockID.
errCode buffer_add_block(block **block_2ptr, const void *data, fileDesc fd,\
		int blockID) {
	if (fdMetaTable[fd - 3].fileName == NULL) { 
		printf("ERROR: fd is not exist\n");
		return 2;
	}
	if (blockID < fdMetaTable[fd - 3].firstBlockID || \
		blockID > fdMetaTable[fd - 3].lastBlockID) {
		printf("ERROR: blockID is not exist\n");
		return 2;
	}

	// loop to find block to replace
	int start = current;
	int m = 0;
	while (m < 2) {

		#ifdef DEBUG
			printf("buffer_add_block, pos = %d\n", current - 1);
		#endif

		if (bufferPool[current - 1].pinCount == 0) {
			if (bufferPool[current - 1].referenced == 0) {
				if (bufferPool[current - 1].data != NULL) {
					free(bufferPool[current - 1].data);
					bufferPool[current - 1].data = NULL;
				}

				// get "freeSpace" from data[]
				int freeSpace[1] = {-1};
				memcpy(freeSpace, data + pageCapacity * (1 + ENTRYLENGTH), \
					sizeof(int));

				init_block(&bufferPool[current - 1]);
				bufferPool[current - 1].freeSpace = freeSpace[0];
				bufferPool[current - 1].pinCount ++;
				bufferPool[current - 1].blockID = blockID;
				bufferPool[current - 1].fd = fd;
				bufferPool[current - 1].referenced = 1;
				bufferPool[current - 1].data = (char *)malloc(FRAMESIZE);
				memcpy(bufferPool[current - 1].data, data, FRAMESIZE);

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
	printf("ERROR: All block in buffer pool is pinned. Cannot replace.\n");
	return 2;
}

// open the first header page of the file, get fd, open metadata page, store
// metadata. If fail, return -1.
fileDesc BM_open_file( const char *filename ) {
	char fileLocation[LOCATIONSIZE];
	get_location(fileLocation, filename, "h0.head");
	FILE *fp = fopen(fileLocation, "rb+");
	if (fp == NULL) { return -1; }
	fileDesc fd = fileno(fp);
	
	//read metadata from metadata page.
	char metaLocation[LOCATIONSIZE];
	get_location(metaLocation, filename, ".meta");
	FILE *pMeta = fopen(metaLocation, "rb");
	if (pMeta == NULL) { return -1; } // no matedata page.
	int meta[4];
	fread(meta, sizeof(int), 4, pMeta);
	fclose(pMeta);
	pMeta = NULL;
	
	// store metadata in direct-address table.
	if (fdMetaTable[fd - 3].fileName != NULL) {
		free(fdMetaTable[fd - 3].fileName);
	}
	init_metadata(&fdMetaTable[fd - 3]);
	fdMetaTable[fd - 3].fileName = (char *)malloc(LOCATIONSIZE);
	strcpy(fdMetaTable[fd - 3].fileName, filename);
	fdMetaTable[fd - 3].firstBlockID = meta[0];
	fdMetaTable[fd - 3].lastBlockID = meta[1];
	fdMetaTable[fd - 3].blockNumber = meta[2];
	fdMetaTable[fd - 3].headerNumber = meta[3];
	fdMetaTable[fd - 3].fp = fp;

	#ifdef DEBUG
		printf("BM_open_file: fd = %d\n", fd);
		printf("firstBlockID: %d\tlastBlockID: %d\tblockNumber: %d\theaderNumber: %d\n",\
		 	meta[0], meta[1], meta[2], meta[3]);
		printf("open file: %s\n", fileLocation);
		printf("*********** BM_open_file ************\n");
	#endif

	return fd;
}

// close file using fd, write metadata, if fails, return 3.
// reset blokcs of this fd in buffer pool
errCode BM_close_file( fileDesc fd ) {
	// scan buffer pool, see if there are still blocks of this file pinned
	for (int i = 0; i < BUFFERSIZE; ++i) {
		if (bufferPool[i].fd == fd) {
			if (bufferPool[i].pinCount != 0) {
				printf("ERROR: BM_close_file, still pinned, cannot close.\n");
				return 3; //still pinned, cannot close.
			}
		}
	}
	// if all blocks in buffer pool of this fd are unpinned, free data memory
	// of them, reset block in buffer pool also
	for (int i = 0; i < BUFFERSIZE; ++i)
	{
		if (bufferPool[i].fd == fd) {
			if (bufferPool[i].data != NULL) {
				free(bufferPool[i].data);
				bufferPool[i].data = NULL;
				bufferPool[i].dirty = 0;
				bufferPool[i].fd = -1;
				bufferPool[i].blockID = INT_MAX;
				bufferPool[i].referenced = 0;
				bufferPool[i].freeSpace = -1;
				bufferPool[i].pinCount = 0;
			}
		}
	}

	// check the input fd if it is valid (still stored in hash table).
	if (fdMetaTable[fd - 3].fileName == NULL) { 
		printf("ERROR:BM_close_file: fd doesn't exist or is closed.\n");		
		return 3; 
	} //already closed.

	// write back metadata to metadata page.
	metadata temp = fdMetaTable[fd - 3];
	char metaLocation[LOCATIONSIZE];
	get_location(metaLocation, temp.fileName, ".meta");
	FILE *fp = fopen(metaLocation, "wb+");
	if (fp == NULL) { 
		printf("ERROR: BM_close_file, cannot open metadata page.\n");
		return 3; 
	} // connot open metafile
	int meta[4] = {temp.firstBlockID, temp.lastBlockID, temp.blockNumber, \
		temp.headerNumber};
	fwrite(meta, sizeof(int), 4, fp);
	fclose(fp);

	//close file using metadata[fd - 3].fp.
	if (fclose(fdMetaTable[fd - 3].fp) != 0) { 
		printf("ERROR: BM_close_file, cannot close fd.\n");
		return 3; 
	}

	//delete fd and metadata from hash table.
	free(fdMetaTable[fd - 3].fileName);
	fdMetaTable[fd - 3].fileName = NULL;
	fdMetaTable[fd - 3].blockNumber = -1;
	fdMetaTable[fd - 3].headerNumber = -1;
	fdMetaTable[fd - 3].firstBlockID = -1;
	fdMetaTable[fd - 3].lastBlockID = -1;
	fdMetaTable[fd - 3].fp = NULL;

	#ifdef DEBUG
		printf("meta page location: %s\n", metaLocation);
		printf("*********** BM_close_file ************\n");
	#endif

	return 0;		
}

// helper function, find the block of specific blockID and fd in buffer pool, 
// if success, return 1, else return 0. Each time of scan begin with current - 1
boolean find_buffer_block(block **block_2ptr, fileDesc fd, int blockID) {
	if (blockID < fdMetaTable[fd - 3].firstBlockID ||\
		blockID > fdMetaTable[fd - 3].lastBlockID) {

		printf("ERROR: find_buffer_block, input blockID doesn't exist.\n");
		return 0;
	}
	for (int i = 0; i < BUFFERSIZE; ++i) {
		if (bufferPool[current - 1].blockID == blockID \
			&& bufferPool[current - 1].fd == fd) {
			
			*block_2ptr = bufferPool + current - 1;
			#ifdef DEBUG
				printf("********* find_buffer_block, success. **********\n");
			#endif

			return 1;
		}
		current = (current == BUFFERSIZE) ? 1 : current + 1;
	}
	#ifdef DEBUG
		printf("********* find_buffer_block, fail. **********\n");
	#endif

	return 0;
}

// helper function, when the block is not in buffer pool, we need to call this
//function to find the block in disk, store result in the data array of FRAMESIZEbytes,
// add this block into buffer pool, store this pointer of new block ,if fails, 
//return errCode 4.
errCode find_disk_block(block **block_2ptr, int blockID, fileDesc fd) {
	metadata temp = fdMetaTable[fd - 3];
	if (blockID < temp.firstBlockID || blockID > temp.lastBlockID \
		|| temp.fileName == NULL) {

		printf("ERROR: find_disk_block, input error.\n");
		return 4; //input block doesn't exist.
	} 
	
	// get location of the disk page 
	char pageLocation[LOCATIONSIZE];
	char IDstring[(int)log10(INT_MAX) + 1];
	sprintf(IDstring, "%d.dat", blockID);
	get_location(pageLocation, temp.fileName, IDstring);

	#ifdef DEBUG
		printf("find_disk_block, get disk location: %s\n", pageLocation);
	#endif

	// read data from disk page into data array
	char data[FRAMESIZE];
	memset(data, 0, FRAMESIZE);
	FILE *fp = fopen(pageLocation, "rb");
	if (fp == NULL) { 
		printf("ERROR: find_disk_block, cannot open, blockID doesn't exist.\n");
		return 4; 
	} // page doesn't exist.
	if (fread(data, 1, FRAMESIZE, fp) != FRAMESIZE) { 
		printf("ERROR: find_disk_block, read error.\n");
		return 4; 
	} // read fail or eof.
	fclose(fp);

	// add this block into buffer pool. Store the pointer of new block in *block_2ptr
	#ifdef DEBUG
		printf("find_disk_block, got data, start add block in buffer.\n");
	#endif
	int err = buffer_add_block(block_2ptr, data, fd, blockID);
	if (err != 0) {
		BM_print_error(err);
		return 4;
	}
	#ifdef DEBUG
		printf("********* find_disk_block **********\n");
	#endif

	return 0;
}

// get first block of a given file, if fails, return 7.
errCode BM_get_first_block( fileDesc fd, block** blockPtr ) {
	// set input constraints first
	if (fd < 3 || fdMetaTable[fd - 3].fileName == NULL) {
		printf("ERROR: BM_get_first_block, input fd:  %d  is not exist.\n", fd);
		return 7;
	}
	// get first blockID from metadata
	int firstID = fdMetaTable[fd - 3].firstBlockID;
	int err = BM_get_this_block(fd, firstID, blockPtr);
	if (err != 0) {
		BM_print_error(err);
		return 7;
	}
	fdMetaTable[fd - 3].currentID = fdMetaTable[fd - 3].firstBlockID;

	#ifdef DEBUG
		printf("*********** BM_get_first_block *************\n");
	#endif
	
	return 0;
}

// get next block according to 'currentID' in metadata, if fails, return 8.
// find next blockID from header files, and using get_this_block to find it.
errCode BM_get_next_block( fileDesc fd, block** blockPtr ) {
	// set input constraints first.
	if (fd < 3 || fdMetaTable[fd - 3].fileName == NULL) {
		printf("ERROR: BM_get_next_block, input fd: %d is not exist.\n", fd);
		return 8;
	}
	if (fdMetaTable[fd - 3].currentID == -1) {
		printf("ERROR: need to set currentID first.\n");
		return 8;
	}
	if (fdMetaTable[fd - 3].currentID == fdMetaTable[fd - 3].lastBlockID) {
		printf("ERROR: BM_get_next_block, this is already the last block.\n");
		return 8;
	}
	// find headerID, and read in header page.
	int blockID = -1;
	int currentID = fdMetaTable[fd - 3].currentID;
	int headerID =  currentID / headerCapacity;
	char IDstr[LOCATIONSIZE], headerLocation[LOCATIONSIZE], data[FRAMESIZE];

	while (headerID < fdMetaTable[fd - 3].headerNumber) {

		#ifdef DEBUG
			printf("currentID = %d, headerID = %d,  headerNum = %d\n", \
				fdMetaTable[fd - 3].currentID, headerID, fdMetaTable[fd - 3].headerNumber);
		#endif

		sprintf(IDstr, "h%d.head", headerID);
		get_location(headerLocation, fdMetaTable[fd - 3].fileName, IDstr);
		FILE *fpHeader = fopen(headerLocation, "rb");
		if (fpHeader == NULL) {
			printf("ERROR: BM_get_next_block, cannot open header file.\n");
			return 8;
		}
		// read header page into data[]
		if (fread(data, 1, FRAMESIZE, fpHeader) != FRAMESIZE) {
			printf("ERROR: BM_get_next_block, fread error.\n");
			return 8;
		}
		fclose(fpHeader);
		fpHeader = NULL;

		int freeSpace[1];
		memcpy(freeSpace, data + headerCapacity, sizeof(int));
		if (freeSpace[0] == headerCapacity) { // if empty, consider next
			headerID ++;
			continue;
		}
		int i = 0;
		for (i = 0; i < headerCapacity; ++i) {
			if (data[i] != -1) {
				if (currentID < (headerCapacity * headerID) + i) {
					blockID = headerCapacity * headerID + i;
					break;
				}
			}
		}
		if (i == headerCapacity) { // no block after current in this header, reach end
			headerID ++;
			continue;
		}
		break; // got blockID.
	}
	if (headerID == fdMetaTable[fd - 3].headerNumber) {
		printf("ERROR: BM_get_next_block, cannot find next, metadata might be wrong\n");
		return 8;
	}
	// we got next blockID.
	int err = BM_get_this_block(fd, blockID, blockPtr);
	if (err != 0) {
		BM_print_error(err);
		return 8;
	}
	fdMetaTable[fd - 3].currentID = blockID;

	#ifdef DEBUG
		printf("*********** BM_get_next_block ************\n");
	#endif

	return 0;
}

// call my helper functions: find_buffer_block() and find_disk_block(),
// if fails, return 5.
errCode BM_get_this_block( fileDesc fd, int blockID, block** blockPtr ) {
	if (fd < 3) {
		printf("ERROR: BM_get_this_block, fd is invalid.\n");
		return 5;
	}
	if (blockID < 0) {
		printf("ERROR: BM_get_this_block, blockID is invalid.\n");
		return 5;
	}
	if (find_buffer_block(blockPtr, fd, blockID)) { 
		// set up metadata.currentID
		fdMetaTable[fd - 3].currentID = blockID;

		#ifdef DEBUG
			printf("********* BM_get_this_block, from buffer **********\n");
		#endif

		return 0; 
	}
	int err = find_disk_block(blockPtr, blockID, fd);
	if (err != 0) {
		BM_print_error(err);
		return 5;
	} else {
		
		#ifdef DEBUG
			printf("********* BM_get_this_block, from disk **********\n");
		#endif
		
		return 0;
	}
}

// create a new block in disk, update metadata as well, 
// need to get a blockID first. Return 6 if fails.
errCode BM_alloc_block( fileDesc fd, int *blockIDptr ) {
	//get a block ID first.
	int blockID = -1, headerID = 0;
	char headerLocation[LOCATIONSIZE] = {0};
	if (fdMetaTable[fd - 3].fileName == NULL) { return 6; } // fd doesn't exist.
	while (headerID < fdMetaTable[fd - 3].headerNumber) {
		// loop to get target header page, and get empty blockID in it.
		// get header location first
		char headerIDStr[LOCATIONSIZE];
		sprintf(headerIDStr, "h%d.head", headerID);
		get_location(headerLocation, fdMetaTable[fd - 3].fileName,\
			headerIDStr);
		
		// open header file, see if there is free space
		FILE *fpHeader = fopen(headerLocation, "rb+");
		if (fpHeader == NULL) { return 6; } // headerID doesn't exist
		char dir[headerCapacity];
		int freeSpace[1];
		char data[FRAMESIZE];
		if (fread(data, 1, FRAMESIZE, fpHeader) != FRAMESIZE) { return 6; } // read error.
		memcpy(dir, data, headerCapacity);
		memcpy(freeSpace, data + headerCapacity, sizeof(int));
		// if no space, consider next header page
		if (freeSpace[0] == 0) { 
			headerID ++; 
			fclose(fpHeader);
			continue; 
		} 
		// if there is free space.
		int i = 0;
		for (i = 0; i < headerCapacity; ++i) {
			// find empty blockID, set it to exist
			if (dir[i] ==  -1) {
				// set counter of the selected slot in directory to "page capacity"
				*((char *)(data + i)) = pageCapacity;
				// "freeSpace" counter - 1
				freeSpace[0] --;
				*((int *)(data + headerCapacity)) = freeSpace[0];
				fclose(fpHeader);
				fpHeader = NULL;
				fpHeader = fopen(headerLocation, "wb+");
				if (fwrite(data, 1, FRAMESIZE, fpHeader) != FRAMESIZE) { return 6; }
				fclose(fpHeader);
				break;
			}
		}
		if (i != headerCapacity) { 
			blockID = headerID * headerCapacity + i;
			break;
		} else {
			return 6; // "freeSpace" counter shows there is space, but cannot find
		}
	}
	// loop end.
	// if there is no free space in header files, need to create new header file
	// initialize new header file, set the first counter to pageCapaticy.
	if (headerID == fdMetaTable[fd - 3]. headerNumber) {
		// get filename of new header file first, known headerID
		char headerIDStr[LOCATIONSIZE] = {0};
		sprintf(headerIDStr, "h%d.head", headerID);
		get_location(headerLocation, fdMetaTable[fd - 3].fileName\
			, headerIDStr);

		// create header file and initialize, update metadata
		FILE *fpHeader = fopen(headerLocation, "wb+");
		if (fpHeader == NULL) { return 6; } // create header file, error
		char data[FRAMESIZE] = {0};
		memset(data, -1, FRAMESIZE - 1 - sizeof(int));
		*((int *)(data + headerCapacity)) = headerCapacity - 1;
		data[0] = pageCapacity;
		data[FRAMESIZE - 1] = '@';
		if (fwrite(data, 1, FRAMESIZE, fpHeader) != FRAMESIZE) { return 6; } // write, error
		fclose(fpHeader);
		fpHeader = NULL;
		fdMetaTable[fd - 3].headerNumber ++;
		blockID = headerID * headerCapacity;
	}

	// we have blockID now, create new page in disk, initialize it, and update
	// metadata.
	char blockIDStr[LOCATIONSIZE] = {0};
	char blockLocation[LOCATIONSIZE] = {0};
	sprintf(blockIDStr, "%d.dat", blockID);
	get_location(blockLocation, fdMetaTable[fd - 3].fileName, blockIDStr);
	FILE *fpBlock = fopen(blockLocation, "wb+");
	if (fpBlock == NULL) { return 6; } // create file, error
	char data1[FRAMESIZE] = {0};
	memset(data1, 0, pageCapacity * ENTRYLENGTH);
	// set directory of each slot in data page, 1 means full, -1 means empty
	memset(data1 + pageCapacity * ENTRYLENGTH, -1, pageCapacity);
	*((int *)(data1 + pageCapacity * (ENTRYLENGTH + 1))) = pageCapacity;
	data1[FRAMESIZE - 1] = '@';
	if (fwrite(data1, 1, FRAMESIZE, fpBlock) != FRAMESIZE) { return 6; } // write, error 
	fclose(fpBlock);
	// update metadata
	if (fdMetaTable[fd - 3].firstBlockID == -1 && \
		fdMetaTable[fd - 3].lastBlockID == -1) {

		fdMetaTable[fd - 3].firstBlockID = 0;
		fdMetaTable[fd - 3].lastBlockID = 0;
	} else if (blockID < fdMetaTable[fd - 3].firstBlockID) {
		fdMetaTable[fd - 3].firstBlockID = blockID;
	} else if (blockID > fdMetaTable[fd - 3].lastBlockID) {
		fdMetaTable[fd - 3].lastBlockID = blockID;
	}
	fdMetaTable[fd - 3].blockNumber ++;

	*blockIDptr = blockID;
	#ifdef DEBUG
		printf("********* BM_alloc_block ***********\n");
	#endif

	return 0;
}
// helper function, find next blockID, if it is the last, return -2, if error, 
// return -1
int get_next_ID(fileDesc fd, int blockID) {
	// there is only this one blockID
	if (blockID == fdMetaTable[fd - 3].lastBlockID) { return -2; }
	int headerID = blockID / headerCapacity;
	int nextID = -1;
	while (headerID < fdMetaTable[fd - 3].headerNumber) {
		char IDstr[LOCATIONSIZE];
		sprintf(IDstr, "h%d.head", headerID);
		char headerLocation[LOCATIONSIZE];
		get_location(headerLocation, fdMetaTable[fd - 3].fileName, IDstr);
		FILE *fp = fopen(headerLocation, "rb");
		if (fp == NULL) { return -1; }
		char data[FRAMESIZE];
		if(fread(data, 1, FRAMESIZE, fp) != FRAMESIZE) { return -1; }
		fclose(fp);
		int i = 0;
		for (i = 0; i < headerCapacity; ++i)
		{	int n = headerID * headerCapacity + i;
			if (data[i] != -1 && n > blockID) {
				nextID = n;
				return nextID;
			}
		}
		// if the loop reach the end
		if (i == headerCapacity) {
			headerID++;
			continue;
		}
		break;
	}
	return -1; 

}

// helper function, find previous blockID, if it is the first, return -2, if
// there is error, return -1
int get_prev_ID(fileDesc fd, int blockID) {
	// there is only this one block
	if (blockID == fdMetaTable[fd - 3].firstBlockID) { return -2; }
	int headerID = blockID / headerCapacity;
	while (headerID >= 0) {
		char IDstr[LOCATIONSIZE];
		sprintf(IDstr, "h%d.head", headerID);
		char headerLocation[LOCATIONSIZE];		
		get_location(headerLocation, fdMetaTable[fd - 3].fileName, IDstr);
		FILE *fp = fopen(headerLocation, "rb");
		if (fp == NULL) { return -1; }
		char data[FRAMESIZE];
		if(fread(data, 1, FRAMESIZE, fp) != FRAMESIZE) { return -1; }
		fclose(fp);
		int i;
		for (i = headerCapacity - 1; i >= 0; --i) {
			int n = headerID * headerCapacity + i;
			if (data[i] != -1 && n < blockID) {
				return n;
			}
		}
		if (i < 0) {
			headerID --;
			continue;
		}
		break;
	}
	printf("hshshhs");
	return -1; 
}

// dispose block, return 9 if it fails
// need to delete block in buffer pool as well as in disk, update metadata 
// and header page.
errCode BM_dispose_block( fileDesc fd, int blockID ) {
	if (fd < 3 || fdMetaTable[fd - 3].fileName == NULL) {
		printf("ERROR: BM_dispose_block, invalid fd: %d\n", fd);
	}
	block *blockPtr = NULL;
	// find the block in buffer pool first, if it is in buffer pool, delete it
	if (find_buffer_block(&blockPtr, fd, blockID)) {
		if (blockPtr->pinCount != 0) {
		// cannot be disposed
		printf("ERROR: BM_dispose_block, still pinned, cannot dispose.\n");
		return 9;
		}
		free(blockPtr->data);
		init_block(blockPtr);
	}
	// get block's location in disk, delete that page in disk
	char location[LOCATIONSIZE] = {0};
	char IDStr[LOCATIONSIZE] = {0};
	sprintf(IDStr, "%d.dat", blockID);
	get_location(location, fdMetaTable[fd - 3].fileName, IDStr);
	if (remove(location) != 0) {
		printf("ERROR: BM_dispose_block, delete file error.\n");
		return 9;
	}

	assert(fdMetaTable[fd - 3].firstBlockID != -1);

	// update metadata
	fdMetaTable[fd - 3].blockNumber --;
	if (fdMetaTable[fd - 3].firstBlockID == fdMetaTable[fd - 3].lastBlockID) {
		fdMetaTable[fd - 3].firstBlockID = -1;
		fdMetaTable[fd - 3].lastBlockID = -1;
		fdMetaTable[fd - 3].currentID = -1;
	} else {
		if (blockID == fdMetaTable[fd - 3].firstBlockID) {
			int firstID = get_next_ID(fd, blockID);
			if (firstID == -1) {
				printf("ERROR: cannot get new firstID.\n");
				return 9;
			} else {
				fdMetaTable[fd - 3].firstBlockID = firstID;
			}
		}
		if (blockID == fdMetaTable[fd - 3].lastBlockID) {
			int lastID = get_prev_ID(fd, blockID);
			if (lastID == -1) {
				printf("ERROR: cannot get new lastID\n");
				return 9;
			} else {
				fdMetaTable[fd - 3].lastBlockID = lastID;
			}
		}
	}
	// update header file
	int headerID = blockID / headerCapacity;
	int offset = blockID % headerCapacity;
	char hIDStr[LOCATIONSIZE];
	sprintf(hIDStr, "h%d.head", headerID);
	char headerLocation[LOCATIONSIZE];
	get_location(headerLocation, fdMetaTable[fd - 3].fileName, hIDStr);
	FILE *pHeader = fopen(headerLocation, "rb+");
	if (pHeader == NULL) {
		printf("ERROR: BM_dispose_block, cannot open header file.\n");
		return 9;
	}
	char data[FRAMESIZE];
	if (fread(data, 1, FRAMESIZE, pHeader) != FRAMESIZE) {
		printf("ERROR: BM_dispose_block, cannot read header file.\n");
	}
	data[offset] = -1;
	*((int *)(data + headerCapacity)) += 1;
	rewind(pHeader);
	if (fwrite(data, 1, FRAMESIZE, pHeader) != FRAMESIZE) {
		printf("ERROR: BM_dispose_block, cannot write back header file.\n");
		return 9;
	}
	fclose(pHeader);

	#ifdef DEBUG
		printf("********** BM_dispose_block ************ \n");
	#endif

	return 0; 
}

// set given block's pinCount to 0 , if fails, return 10.
errCode BM_unpin_block( block* blockPtr ) {
	if (blockPtr->dirty == 1) {
		// get block's disk location to write back
		int fd = blockPtr->fd;
		char pageLocation[LOCATIONSIZE] = {0};
		char IdString[LOCATIONSIZE];
		sprintf(IdString, "%d.dat", blockPtr->blockID);				
		get_location(pageLocation, fdMetaTable[fd - 3].fileName, IdString);

		#ifdef DEBUG
			printf("pageLocation of the block needed to unpin and write back: %s\n",\
				 pageLocation);
		#endif

		// write back current block to disk, update freeSpace as well
		int freeSpace[1] = {blockPtr->freeSpace};
		memcpy(blockPtr->data + pageCapacity * (ENTRYLENGTH + 1),\
			freeSpace, sizeof(int));

		FILE *fp = fopen(pageLocation, "wb+");
		if (fp == NULL) { 
			printf("ERROR: open file fails, cannot write back old block\n");
			return 10; 
		}
		int len = fwrite(blockPtr->data, 1, FRAMESIZE, fp);
		if (len != FRAMESIZE) { 
				printf("ERROR: cannot write back block ");					
			return 10; 
		}
		fclose(fp);
	}
	// unpin
	blockPtr->pinCount = 0;
	blockPtr->dirty = 0;

	return 0;
}

void BM_print_error( errCode ec ) {
	if (ec == 0) { return; }
	fprintf(stderr,     "|-----------------------------------------|\n");
	fprintf(stderr,     "|             ERROR MESSAGES              |\n");
	if (ec == 1) { 
		fprintf(stderr, "|           ERROR: BM_create_file         |\n");
	}
	if (ec == 2) {
		fprintf(stderr, "|          ERROR: Buffer_add_block        |\n");
	}
	if (ec == 3) {
		fprintf(stderr, "|           ERROR: BM_close_file          |\n");
	}
	if (ec == 4) {
		fprintf(stderr, "|          ERROR: find_disk_block         |\n");
	}
	if (ec == 5) {  
		fprintf(stderr, "|         ERROR: BM_get_this_block        | \n");
	}
	if (ec == 6) {
		fprintf(stderr, "|          ERROR: BM_alloc_block          |\n");
	}	
	if (ec == 7) {
		fprintf(stderr, "|         ERROR: BM_get_first_block       |\n");
	}
	if (ec == 8) {
		fprintf(stderr, "|         ERROR: BM_get_next_block        |\n");
	}
	if (ec == 9) {
		fprintf(stderr, "|         ERROR: BM_dispose_block         |\n");
	}
	if (ec == 10) {
		fprintf(stderr, "|          ERROR: BM_unpin_block          |\n");
	}
	fprintf(stderr,     "|_________________________________________|\n");
}
























