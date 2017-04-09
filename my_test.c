#include <assert.h>
#include <memory.h>
#include "projectA.h"

void test_init_block(void);
void test_BM_init(void);
void test_BM_create_file(const char *filename);
void test_BM_open_file(void);
void test_buffer_add_block(void);
void test_BM_close_file(void);


int main(void) {
    printf("############ linking successful ###########\n");

    // test_init_block();
    // test_BM_init();
    // char str[] = "t1";
    // test_BM_create_file(str);
    // test_BM_open_file();
    // test_buffer_add_block();
    test_BM_close_file();

    return 0;
}

void test_BM_close_file() {
    BM_init();
    char str[] = "t1";
    BM_create_file(str);
    BM_open_file(str);

    printf("before: metatable: %d %d %s\n", fdMetaTable[0].blockNumber,\
        fdMetaTable[0].headerNumber, fdMetaTable[0].fileName); 
    
    if (BM_close_file(4) != 0) { exit(3); }

    printf("aftert: metatable: %d %d %s\n", fdMetaTable[0].blockNumber,\
        fdMetaTable[0].headerNumber, fdMetaTable[0].fileName);    
}

void test_buffer_add_block() {
    BM_init();
    bufferPool[0].pinCount = 0;
    bufferPool[0].referenced = 1;
    bufferPool[1].pinCount = 0;
    bufferPool[1].referenced = 0;
    bufferPool[2].pinCount = 0;
    bufferPool[2].referenced = 1;    
    char *data = (char *)malloc(4096);
    data[0] = '1';
    data[4095] = '@';
    char *pageLocation = (char *)malloc(LOCATIONSIZE);
    strcpy(pageLocation, "./data/hahaha.dat");
    printf("%c\n%c\n%s\n", data[0], data[4095], pageLocation);

    block *blockPtr;
    int err = buffer_add_block(&blockPtr, data, 0, 0, pageLocation);
    if (err != 0) { exit(err); }
    printf("%c\n%c\n%s\n", blockPtr->data[0], blockPtr->data[4095],\
        blockPtr->pageLocation);
    printf("The index selected to replace is: %ld\n", blockPtr - bufferPool);
}

void test_BM_open_file() {
    BM_init();
    char str[] = "t1";
    BM_create_file(str);
    BM_open_file(str);
    printf("test: metatable: %d %d %s\n", fdMetaTable[0].blockNumber,\
        fdMetaTable[0].headerNumber, fdMetaTable[0].fileName);

}

void test_BM_create_file(const char *filename) {
    int a = BM_create_file(filename);
    fprintf(stderr, "%d\n", a);
    // should create a t1_-1.dat file in ./data/
}

void test_BM_init() {
    BM_init();
    for (int i = 0; i < BUFFERSIZE; ++i) {
    printf("%d ", bufferPool[0].pinCount);
    printf("%d ", bufferPool[0].dirty);
    printf("%d ", bufferPool[0].fd);
    printf("%d ", bufferPool[0].blockID);
    printf("%d ", bufferPool[0].referenced);
    printf("%d ", bufferPool[0].freeSpace);
    printf("%p ", bufferPool[0].pageLocation);
    printf("%p\n", bufferPool[0].data);        
    }
    //should print each value of each buffer block.

    printf("max_fd = %llu\n", max_fd);
    // should print the max number of fd.

    printf("%p\n", fdMetaTable);
    metadata meta = fdMetaTable[max_fd];
    printf("%p %d\n", meta.fileName, meta.blockNumber);
    // should print 0X0 and 0.


}

// void test_init_block() {
//     block* blockArray = (block*)malloc(10 * sizeof(block));
//     printf("%ld ", blockArray[0].pinCount);
//     printf("%ld ", blockArray[0].dirty);
//     printf("%ld ", blockArray[0].fd);
//     printf("%ld ", blockArray[0].blockID);
//     printf("%ld ", blockArray[0].referenced);
//     printf("%ld ", blockArray[0].freeSpace);
//     printf("%ld ", blockArray[0].pageLocation);
//     printf("%ld\n", blockArray[0].data);
// // should print eight 0's
//     init_block(blockArray);
//     printf("%ld ", blockArray[0].pinCount);
//     printf("%ld ", blockArray[0].dirty);
//     printf("%d ", blockArray[0].fd);
//     printf("%ld ", blockArray[0].blockID);
//     printf("%ld ", blockArray[0].referenced);
//     printf("%d ", blockArray[0].freeSpace);
//     printf("%ld ", blockArray[0].pageLocation);
//     printf("%ld\n", blockArray[0].data);    
// // should print 0 0 -1 0 1 -1 0 0 
// }




































