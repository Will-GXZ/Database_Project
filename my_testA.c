#include <assert.h>
#include <memory.h>

#include "projectA.h"



void populate_block(block* blockPtr);

void test_init_block(void);
void test_BM_init(void);
void test_BM_create_file(const char *filename);
void test_BM_open_file(void);
void test_buffer_add_block(void);
void test_BM_close_file(void);
void test_find_buffer_block(void);
void test_find_disk_block(void);
void show_fdMetaTable(void);
void show_bufferPool(void);
void test_get_this_block(void);
void test_BM_alloc_block(void);
void test_BM_alloc_block10000(void);
void test_get_first_block(void);
void test_get_next_block(void);
void test_BM_unpin_block(void);
void test_get_next_ID(void);
void test_get_prev_ID(void);
void test_BM_dispose_block(void);
int given_test_main(void);
void Memory_test(void);


int main(void) {
    printf("############ linking successful ###########\n");

    /************************************************************** 
     *                                                            *
     *    Unit Tests: Test boundary cases for each function       *
     *                                                            *
     *                                                            *
     **************************************************************/

    // test_BM_init();
    // test_BM_create_file("t1");
    // test_BM_open_file();
    // test_buffer_add_block();
    // test_BM_close_file();
    // test_find_buffer_block();
    // test_find_disk_block();
    // test_get_this_block();
    // test_BM_alloc_block();
    // test_BM_alloc_block10000();
    // test_get_first_block();
    // test_get_next_block();
    // test_BM_unpin_block();
    // test_get_next_ID();
    // test_get_prev_ID();
    // test_BM_dispose_block();

   /*****************************************************************
    *                                                               *
    *                     Memory Test:                              *
    *           Test if there is memory leak                        *
    *           Need to run test with:                              *
    *       valgrind --leak-check=full --show-leak-kinds=all \      *
    *                --track-origins=yes'                           *
    *****************************************************************/

    // Memory_test();


    //****************************************************************
    //*                                                              *
    //*            This is the given testmain function.              *
    //*    added with some code to show results and error messages   *
    //*                                                              *
    //****************************************************************
    
    given_test_main();

    return 0;
}

void Memory_test() {
    int err = 0;
    block *blockPtr = NULL;

    // initialize buffer manager
    BM_init();

    printf("======== Test1: create files and allocate new blocks =======\n");
    // create 3 files, allock 10000 pages for each file
    BM_create_file("test1");
    BM_create_file("test2");
    BM_create_file("test3");
    fileDesc fds[3];
    fds[0] = BM_open_file("test1");
    fds[1] = BM_open_file("test2");
    fds[2] = BM_open_file("test3");
    for (int i = 0; i < 10000; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            err = BM_alloc_block(fds[j]);
            BM_print_error(err);
            assert(err == 0);
        }
    }
    show_bufferPool();
    show_fdMetaTable();

    printf("============ Test2: dispose 10000 blocks * 3 =============\n");
    //  use get next to get each block, after call get_next_block, the block
    //  will be added in buffer pool, need to unpin first
    for (int i = 0; i < 3; ++i) // get_first, set currentID of each file to 0.
    {
        err = BM_get_first_block(fds[i], &blockPtr);
        BM_print_error(err);
        assert(err == 0);
        blockPtr->dirty = 1;  // set dirty to 1, so we can test for write back
        err = BM_unpin_block(blockPtr);
        BM_print_error(err);
        assert(err == 0);
        err = BM_dispose_block(fds[i], blockPtr->blockID);
        BM_print_error(err);
        assert(err == 0);
    }

    for (int i = 0; i < 9999; ++i)
    {   
        for (int j = 0; j < 3; ++j)
        {
            err = BM_get_next_block(fds[j], &blockPtr);
            BM_print_error(err);
            if (err != 0) { show_fdMetaTable(); }
            assert(err == 0);
            blockPtr->dirty = 1;  // set dirty to 1, so we can test writing back
            err = BM_unpin_block(blockPtr);
            BM_print_error(err);
            assert(err == 0);
            err = BM_dispose_block(fds[j], blockPtr->blockID);
            BM_print_error(err);
            assert(err == 0);
        }
    }
    show_fdMetaTable();
    show_bufferPool();
    for (int i = 0; i < 3; ++i)
    {
        err = BM_close_file(fds[i]);
        BM_print_error(err);
        assert(err == 0);
    }
    show_fdMetaTable();


    free(bufferPool);
    free(fdMetaTable);
}

// this is the given test main function
int given_test_main(void) {
    int err = 0;
    /*
    bm init
    */
    BM_init();

    printf("===================  Given test: 1 =====================\n");
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

    printf("===================  Given test: 2 =====================\n");
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
    err = BM_get_first_block(fd, &blockPtr);
    BM_print_error(err);
    populate_block(blockPtr);
    blockPtr->dirty=1; //we just made changes
    err = BM_unpin_block(blockPtr);
    BM_print_error(err);
    assert(blockPtr->pinCount==0); //pin count should be 0  
    for (i = 0; i < 99; i++) {
        err = BM_get_next_block(fd, &blockPtr);
        BM_print_error(err);
        populate_block(blockPtr);
        blockPtr->dirty=1; //we just made changes
        err = BM_unpin_block(blockPtr);
        BM_print_error(err);
        assert(blockPtr->pinCount==0); //pin count should be 0  
    }

    err = BM_get_first_block(fd, &blockPtr);
    BM_print_error(err);

    //we do not expect to close a file with pinned blocks
    err = BM_unpin_block(blockPtr);
    BM_print_error(err);
    assert(blockPtr->pinCount==0); //pin count should be 0  
    for (i = 0; i < 99; i++) {
        err = BM_get_next_block(fd, &blockPtr);
        BM_print_error(err);
        //we do not expect to close a file with pinned blocks
        err = BM_unpin_block(blockPtr);
        BM_print_error(err);
        assert(blockPtr->pinCount==0); //pin count should be 0  
    }

    err = BM_close_file(fd);
    BM_print_error(err);
    show_bufferPool();
    show_fdMetaTable();
    // file should have 100 blocks now

    printf("===================  Given test: 3 =====================\n");
    /*
    open the above file 
    access pages
    close the file
    */
    char testing_elements[50];
    fd = BM_open_file("testing.dat");
    for (i = 0; i < 50; i++) {
        err = BM_get_this_block(fd, (i * i + i) % 100, &blockPtr);
        BM_print_error(err);
        populate_block(blockPtr);
        testing_elements[i]=blockPtr->data[i];
        //we do not expect to close a file with pinned blocks
        blockPtr->dirty=1;
        err = BM_unpin_block(blockPtr);
        BM_print_error(err);
        assert(blockPtr->pinCount==0); //pin count should be 0  
    }
    err = BM_close_file(fd);
    BM_print_error(err);
    show_bufferPool();
    show_fdMetaTable();

    printf("===================  Given test: 4 =====================\n");
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
    show_bufferPool();
    show_fdMetaTable();
    BM_close_file(fd);

    printf("===================  Given test: 5 =====================\n");
    /*
    open the above file 
    remove pages
    close the file
    */
    fd = BM_open_file("testing.dat");
    for (i = 0; i < 100; i++) {
        err = BM_dispose_block(fd, i);
        BM_print_error(err);
    }
    err = BM_close_file(fd);
    BM_print_error(err);
    show_bufferPool();
    show_fdMetaTable();

    printf("=================  Given test: 6 ==================\n");
    printf("______________________________________________________________\n");
    printf("Should print error message,");
    printf(" because now there is no page in this file\n");
    printf("______________________________________________________________\n");
    /*
    bm open speficic file (checks that its opening an existing file not making a new one)
    bm get first block
    bm dispose block
    bm close file
    */

    fd = BM_open_file("testing.dat");
    err = BM_get_first_block(fd, &blockPtr);
    BM_print_error(err);
    err = BM_unpin_block(blockPtr);
    BM_print_error(err);
    assert(blockPtr->pinCount==0); //pin count should be 0  
    err = BM_dispose_block(fd, 0);
    BM_print_error(err);
    err = BM_close_file(fd);
    BM_print_error(err);    
    /*
    all throughout print some errors if there are any.
    */

    return 0;    
}


void test_BM_dispose_block() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1"); 
    for (int i = 0; i < 10000 ; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }
    printf("=========== TEST: dispose 10000 block ===========\n"); 
    show_fdMetaTable();
    show_bufferPool();
    int a;
    scanf("%d", &a);
    for (int i = 0; i < 10000; ++i)
    {
        err = BM_dispose_block(fd, i);
        BM_print_error(err);
    }
    show_fdMetaTable();
    show_bufferPool();
    BM_close_file(fd);
}

void test_get_prev_ID() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");\
    for (int i = 0; i < 10000 ; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }
    printf("=========== TEST: get prev ID ===========\n"); 
    for (int i = 9999; i >=0 ; --i)
    {
        int prev = get_prev_ID(fd, i);
        printf("%d ", prev);
    }
}

void test_get_next_ID() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");

    for (int i = 0; i < 10000 ; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }
    printf("=========== TEST: get next ID ===========\n"); 
    for (int i = 0; i < 10000; ++i) {
        int next =  get_next_ID(fd, i);
        printf("%d ", next);
    }
       
}

void test_BM_unpin_block() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    block *blockPtr = NULL;

    for (int i = 0; i < 10 ; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }
    printf("========== TEST: unpin block ===========\n");
    for (int i = 0; i < 5; ++i)
    {
        err = BM_get_this_block(fd, i, &blockPtr);
        BM_print_error(err);
        assert(err == 0);
    }
    for (int i = 0; i < 5; ++i)
    {
        bufferPool[i].dirty = 1;
        bufferPool[i].freeSpace = 10;
    }
    show_bufferPool();
    show_fdMetaTable();
    for (int i = 0; i < 5; ++i)
    {
        err = BM_unpin_block(&bufferPool[i]);
        BM_print_error(err);
        assert(err == 0);
    }
    show_bufferPool();
    show_fdMetaTable();

}

void test_get_next_block() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");

    for (int i = 0; i < 5000; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }
    block *blockPtr = NULL;

    printf("============== TEST: next doesn't exist ==============\n");
    // currentID is the lastID
    fdMetaTable[fd - 3].currentID = 4999;
    err = BM_get_next_block(fd, &blockPtr);
    BM_print_error(err);
    show_fdMetaTable();
    show_bufferPool();

    printf("============== TEST: sequential get block ==============\n");
    BM_get_first_block(fd, &blockPtr);
    for (int j = 1; j < 5000; ++j) {
        for (int i = 0; i < BUFFERSIZE; ++i) {
                bufferPool[i].pinCount = 0;
        } // allow continuously add to buffer pool
        err = BM_get_next_block(fd, &blockPtr);
        BM_print_error(err);
        assert(err == 0);
        assert(blockPtr->blockID == j);
    }    
    show_fdMetaTable();
    show_bufferPool();
}

void test_get_first_block() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");

    for (int i = 0; i < 100; ++i)
    {
        printf("%d ", i);
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
    }

    printf("================ TEST: get first block ================\n");
    block *blockPtr = NULL;
    err = BM_get_first_block(fd, &blockPtr);
    BM_print_error(err);
    printf("the ID of the first block that we get is: %d\n", blockPtr->blockID);
    show_fdMetaTable();
    show_bufferPool();
}

// continuiously allocate 20 000 new block
void test_BM_alloc_block10000() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    show_fdMetaTable();

    for (int i = 0; i < 20000; ++i) {
        err = BM_alloc_block(fd);
        BM_print_error(err);
        assert(err == 0);
        printf("            %d\n", i);
    }
    show_fdMetaTable();

    char data[FRAMESIZE];
    FILE *fp = fopen("./data/t1_h4.head", "rb");
    fread(data, 1, FRAMESIZE, fp);
    int a[1];
    memcpy(a, data + headerCapacity, sizeof(int));
    printf("%d\n", a[0]);
}

void test_BM_alloc_block() {
    int err = 0;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    show_fdMetaTable();

    // test for invalid fd.
    printf("\n\n=============== test for invalid fd ================\n\n");
    err = BM_alloc_block(4);
    BM_print_error(err);

    // test: 1st header page has free space
    printf("\n\n======== test: 1st header file has free space =======\n\n");
    err = BM_alloc_block(fd);
    BM_print_error(err);
    show_fdMetaTable();

    // test: 1st header is full, 2nd has free space
    // set 1st header to full, 2nd empty
    printf("\n\n======== test: 1st is full, 2nd has free space =======\n\n");
    fdMetaTable[fd - 3].headerNumber = 2;
    char data[FRAMESIZE];
    memset(data, 1, headerCapacity);
    *((int *)(data + headerCapacity)) = 0;
    data[FRAMESIZE - 1] = '@';
    FILE *fp = fopen("./data/t1_h0.head", "wb+");
    if (fp == NULL) { exit(1);}
    if(fwrite(data, 1, FRAMESIZE, fp) != FRAMESIZE) { exit(2); }
    fclose(fp);
    fp = NULL;
    fp = fopen("./data/t1_h1.head", "wb+");
    if (fp == NULL) { exit (3); }
    memset(data, -1, FRAMESIZE - 1 - sizeof(int));
    *((int *)(data + headerCapacity)) = headerCapacity - 1;
    if (fwrite(data, 1, FRAMESIZE, fp) != FRAMESIZE) { exit(3); } // write, error
    fclose(fp);
    fp = NULL;

    err = BM_alloc_block(fd);
    BM_print_error(err);
    show_fdMetaTable();

    // test: 1st, 2nd are full, need to create new header file
    printf("\n\n===== test: 1,2 are full, need to create new header =====\n\n");
    memset(data, 1, headerCapacity);
    *((int *)(data + headerCapacity)) = 0;
    data[FRAMESIZE - 1] = '@';
    fp = fopen("./data/t1_h1.head", "wb+");
    if (fp == NULL) { exit(1);}
    if(fwrite(data, 1, FRAMESIZE, fp) != FRAMESIZE) { exit(2); }
    fclose(fp);
    fp = NULL;

    err = BM_alloc_block(fd);
    BM_print_error(err);
    show_fdMetaTable();

}

void test_get_this_block() {
    int err;
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    // create new blocks to test
    for (int i = 0; i < 100; ++i)
    {
        err = BM_alloc_block(fd);
        BM_print_error(err);
    }
    show_fdMetaTable();
    show_bufferPool();

    block *blockPtr = NULL;

    // test: non-exist fd
    printf("============= TEST: fd that doesn't exist ===============\n");
    err = BM_get_this_block(4, 0, &blockPtr);
    BM_print_error(err);
    if (err == 0) {
        printf("the ID of block that we get is: %d\n", blockPtr->blockID);
    }
    // test: non-exist blockID
    printf("=========== TEST: blockID that doesn't exitst ============\n");
    err = BM_get_this_block(fd, 100, &blockPtr); // we only have 0 ~ 99.
    BM_print_error(err);
    if (err == 0) {
        printf("the ID of block that we get is: %d\n", blockPtr->blockID);
    }
    // test: get block from disk
    printf("=========== TEST: block is not in buffer ============\n");
    show_fdMetaTable();
    show_bufferPool();
    err = BM_get_this_block(fd, 5, &blockPtr);
    BM_print_error(err);
    if (err == 0) {
        printf("the ID of block that we get is: %d\n", blockPtr->blockID);
    }
    show_fdMetaTable();
    show_bufferPool();
    //test: block is in buffer
    printf("=========== TEST: block is in buffer pool ============\n");
    err = BM_get_this_block(fd, 5, &blockPtr);
    BM_print_error(err);
    if (err == 0) {
        printf("the ID of block that we get is: %d\n", blockPtr->blockID);
    }
    show_fdMetaTable();
    show_bufferPool();
}

void test_find_disk_block() {
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    printf("fd = %d\n", fd);
    show_fdMetaTable();
    show_bufferPool();

    block *blockPtr = NULL;
    // test unsuccessful finding 
    int err = find_disk_block(&blockPtr, 3, fd); 
    BM_print_error(err);    //this error is caused by metadata.
    
    fdMetaTable[fd - 3].lastBlockID = 3;
    err = find_disk_block(&blockPtr, 3, fd);
    BM_print_error(err);  //this error is due to file page with ID = 3 doesn't exist

    // create a page with ID = 3 for 't1'
    char data[FRAMESIZE];
    memset(data, 0, FRAMESIZE);
    data[0] = 120;
    data[FRAMESIZE - 1] = 89;
    char location[20];
    get_location(location, "t1", "3.dat");
    FILE *fp = fopen(location, "wb+");
    if (fwrite(data, 1, FRAMESIZE, fp) != FRAMESIZE) { printf("file error\n"); }
    fclose(fp);
    err = find_disk_block(&blockPtr, 3, fd);
    BM_print_error(err); // this time, find successfully
    show_bufferPool();

}

void show_fdMetaTable() {
    printf("\n!!!!!!!!!!! show_fdMetaTable !!!!!!!!!!!!\n\n");
    for (int i = 0; i < max_fd; ++i) {
        if (fdMetaTable[i].fileName != NULL) {
            printf("fd: %d,  ", i + 3);
            printf("fileName: %s,  ", fdMetaTable[i].fileName);
            printf("firstID: %d, lastID: %d, blockNum: %d, headerNum: %d, ", \
                fdMetaTable[i].firstBlockID, fdMetaTable[i].lastBlockID,\
                fdMetaTable[i].blockNumber, fdMetaTable[i].headerNumber);
            printf("currentID: %d, fp: %p\n", fdMetaTable[i].currentID,\
                 fdMetaTable[i].fp);
            printf("-------------------------------------------------------\n");
        }
    }
}

void show_bufferPool() {
    printf("\n!!!!!!!!!!! show_bufferPool !!!!!!!!!!!!\n\n");
    for (int i = 0; i < BUFFERSIZE; ++i) {
        printf("BufferIndex: %d, blockID: %d, pinCount: %d, dirty: %d, ", i, \
            bufferPool[i].blockID, bufferPool[i].pinCount, bufferPool[i].dirty);
        printf("referenced: %d, freeSpace: %d, fd: %d, data: %p\n", \
            bufferPool[i].referenced, bufferPool[i].freeSpace, \
            bufferPool[i].fd, bufferPool[i].data);
        printf("-----------------------------------------------------------\n");
    }
}

void test_find_buffer_block() {
    BM_init();
    char str[] = "t1";
    BM_create_file(str);
    BM_open_file(str);
    bufferPool[2].fd = 3;
    bufferPool[2].blockID = 6;
    block *blockPtr = NULL;
    if (find_buffer_block(&blockPtr, 3, 6) == 0) {
        printf("didn't find this block in buffer pool.");
    } else {
        printf("successfully found block: %d  %d\n", blockPtr->fd, blockPtr->blockID);
    }
}

void test_BM_close_file() {
    BM_init();
    char str[] = "t1";
    BM_create_file(str);
    BM_open_file(str);

    printf("before: metatable: %d %d %s\n", fdMetaTable[0].blockNumber,\
        fdMetaTable[0].headerNumber, fdMetaTable[0].fileName); 
    
    if (BM_close_file(3) != 0) { exit(3); }

    printf("aftert: metatable: %d %d %s\n", fdMetaTable[0].blockNumber,\
        fdMetaTable[0].headerNumber, fdMetaTable[0].fileName);    
}

void test_buffer_add_block() {
    //Modify attributes of blocks to test
    BM_init();
    BM_create_file("t1");
    int fd = BM_open_file("t1");
    show_fdMetaTable();
    int err = 0;

    // create new blocks to test
    for (int i = 0; i < 100; ++i)
    {
        err = BM_alloc_block(fd);
        BM_print_error(err);
    }

    //test: buffer poll is empty, add 3 pages.
    show_fdMetaTable();
    printf("\n\n======= TEST: Buffer Pool is empty, add 3 pages =======\n\n");
    for (int i = 0; i < 3; ++i)
    {
        int blockID = i;
        char location[LOCATIONSIZE];
        char blockIDStr[LOCATIONSIZE];
        sprintf(blockIDStr, "%d.dat", blockID);
        get_location(location, fdMetaTable[fd - 3].fileName, blockIDStr);
        FILE *fp = fopen(location, "rb+");
        assert(fp != NULL);
        char data[FRAMESIZE];
        if (fread(data, 1, FRAMESIZE, fp) != FRAMESIZE) { exit(1); };
        fclose(fp);
        block *blockPtr = NULL;
        printf("adding blockID: %d\n", blockID);
        err = buffer_add_block(&blockPtr, data, fd, blockID);
        BM_print_error(err);
        if (err != 0) {
            show_fdMetaTable();
            show_bufferPool();
            break;
        }
        
    }
    show_fdMetaTable();
    show_bufferPool();

    //test: try to add 20 more pages which is more than buffer blocks.
    printf("\n\n======= TEST: try to add 20 more pages =======\n\n");
    for (int i = 0; i < 20; ++i)
    {
        int blockID = i;
        char location[LOCATIONSIZE];
        char blockIDStr[LOCATIONSIZE];
        sprintf(blockIDStr, "%d.dat", blockID);
        get_location(location, fdMetaTable[fd - 3].fileName, blockIDStr);
        FILE *fp = fopen(location, "rb+");
        assert(fp != NULL);
        char data[FRAMESIZE];
        if (fread(data, 1, FRAMESIZE, fp) != FRAMESIZE) { exit(1); };
        fclose(fp);
        block *blockPtr = NULL;
        printf("adding blockID: %d\n", blockID);
        err = buffer_add_block(&blockPtr, data, fd, blockID);
        BM_print_error(err);
        if (err != 0) {
            break;
        }
    }
    show_fdMetaTable();
    show_bufferPool();
}

void test_BM_open_file() {
    BM_init();
    char str[] = "t1";
    BM_create_file(str);
    int fd = BM_open_file(str);
    printf("returned fd = %d\n", fd);
    printf("test: metatable: %d %d %s\n", fdMetaTable[fd - 3].blockNumber,\
        fdMetaTable[fd - 3].headerNumber, fdMetaTable[fd - 3].fileName);

}

void test_BM_create_file(const char *filename) {
    BM_init();
    int err = BM_create_file(filename);
    BM_print_error(err);
    // should create a t1_-1.dat file in ./data/
}

void test_BM_init() {
    BM_init();
    for (int i = 0; i < BUFFERSIZE; ++i) {
    printf("pin: %d ", bufferPool[0].pinCount);
    printf("dirty: %d ", bufferPool[0].dirty);
    printf("fd: %d ", bufferPool[0].fd);
    printf("blockID: %X ", bufferPool[0].blockID);
    printf("referenced: %d ", bufferPool[0].referenced);
    printf("freeSpace: %d ", bufferPool[0].freeSpace);
    printf("data: %p\n", bufferPool[0].data);        
    }
    //should print each value of each buffer block.

    printf("max_fd = %llu\n", max_fd);
    // should print the max number of fd.

    printf("metaTable: %p\n", fdMetaTable);
    metadata meta = fdMetaTable[max_fd - 1];
    printf("%p %d\n", meta.fileName, meta.blockNumber);
    // should print 0X0 and -1.


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




































