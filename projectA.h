#include <stdio.h>
#include <stdlib.h>

#ifndef PROJECTA_H
#define PROJECTA_H

#define FRAMESIZE 4096
#define BUFFERSIZE 5
#define LOCATIONSIZE 30
#define INT_MAX 0x7fffffff
#define ENTRYLENGTH 40

/*The key for accessing a file's metadata,
 *like its physical location, # blocks, ...*/
typedef int boolean;
typedef int fileDesc;
typedef int errCode; 
typedef struct _block{
    int pinCount;
    int dirty;
    char* data; // void data[FRAMESIZE];
    fileDesc fd;
    //start by 0, header pages do not enter buffer pool. 
    int blockID; 
    int referenced;//used for buffer replacement policy.
    int freeSpace; //in terms of #entries.
} block;


typedef struct _metadata {
    int currentID; // the blockID that is current in use
    int firstBlockID;
    int lastBlockID;
    int blockNumber; // the number of total page the file has
    int headerNumber; // the number of header pages of the file.
    char *fileName;
} metadata;

// global variable used for buffer pool replacement policy. 
// From 1 to BUFFERSIZE.
extern int current;

// an array of blocks, body of the buffer pool.
extern block* bufferPool;  

// a direct-address table to store fd-metadata pairs.
extern metadata* fdMetaTable;

// the max number of file descriptor that the system 
// can assign for this process minus 3.
extern long long max_fd;

// the number of entries a page can contain
extern int pageCapacity;

// the number of pageID's a header file can contain
extern int headerCapacity;

/*Perform necessary initializations for the buffer layer. 
 *This probably involves creating a buffer pool with a specified
 *size and frame size (you can make these parameters to the function
 *or define them as constants in your code), and initializing bookkeeping
 *variables for each frame in the pool.
 */
void BM_init();

/*Create a new DB file on disk.
 *You can use a C library function to help you with this. 
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_create_file( const char *filename );

/*Open an existing file and return a file descriptor for it.
 *A C library function will help you open the file.
 */
fileDesc BM_open_file( const char *filename ); 

/*Close an opened file and return 0 if the operation succeeds
 *or an error code if it fails. 
 *Again, you can use a C library function for this.
 */
errCode BM_close_file( fileDesc fd );

/*Make blockPtr point to the buffer pool location of the file's first block.
 *If the block does not exist in the buffer pool yet, read it in from the file.
 *This might involve replacing a block in the buffer pool if all frames 
 *are full.
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_get_first_block( fileDesc fd, block** blockPtr );

/*Make blockPtr point to the buffer pool location of the file's next block.
 *Again, this might involve replacing a block in the buffer pool if all frames
 *are full.
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_get_next_block( fileDesc fd, block** blockPtr );

/*Make blockPtr point to the buffer pool location of the block with ID blockID.
 *Again, this might involve replacing a block in the buffer pool if all frames
 *are full. 
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_get_this_block( fileDesc, int blockID, block** blockPtr );

/*Add a new block to an open file and save the new block's ID.
 *This means both allocating an extra on-disk block for the file 
 *and updating the metadata for that file. Return 0 if the operation
 *succeeds or an error code if it fails.
 */
errCode BM_alloc_block( fileDesc fd );


/*Remove the block with ID blockID from an open file.
 *This means both freeing disk space and updating the file's metadata.
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_dispose_block( fileDesc, int blockID );


/*Mark the block that blockPtr points to as not in use,
 *so that it can be removed when the buffer pool gets full and a block has
 *to be replaced. When a block is unpinned, the buffer manager should write
 *it to disk immediately so that it doesn't have to keep track of which blocks
 *are dirty (this makes bookkeeping simpler). 
 *Return 0 if the operation succeeds or an error code if it fails.
 */
errCode BM_unpin_block( block* blockPtr );

/*Print the error message that corresponds to the error code.
 */
void BM_print_error( errCode ec );


// helper functions, put them here just for testing
void init_block(block* blockPtr);
void init_metadata(metadata* metaPtr);
void get_location(char *str, const char *filename, const char *postfix);
errCode buffer_add_block(block **block_2ptr, const void *data, fileDesc fd,\
    int blockID);
boolean find_buffer_block(block **block_2ptr, fileDesc fd, int blockID);
errCode find_disk_block(block **block_2ptr, int blockID, fileDesc fd);
int get_next_ID(fileDesc fd, int blockID);
int get_prev_ID(fileDesc fd, int blockID);



#endif


























