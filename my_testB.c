/*  Comp115 Final Project, 2017 Spring
 *  
 *  UserName:    XGUO04
 *  StudentName: Xiaozheng Guo
 *  Created at:  April, 2017
*/

#include <assert.h>
#include <string.h>
#include "projectA.h"
#include "projectB.h"



// Helper functions for testing                

record mkRecord(int attr1, int attr2, int attr3,
        char attr4, char attr5, char attr6, char attr7);
void show_fdMetaTable(void);
void show_bufferPool(void);
void unpin_all(void);
void print_rec(record);
void pause(void);


// Test functions.

void test_HFL_init(void);
void test_HFL_create_file(void);
void test_HFL_open_file(void);
void test_HFL_close_file(void);
void test_get_mapArray(void);
void test_HFL_insert_rec(void); // together with delete_rec and open
void test_HFL_get_first_rec(void);
void test_HFL_get_next_rec(void);
void test_HFL_get_this_rec(void);
// test 3 scanner functions
void test_scanner(void);


/////////////////////////////////
//    This is the given test   //
/////////////////////////////////
void given_test_function(void);



int main(void) {
    
/********************************************************************/
//                                                                  //
//  Unit test functions, some of them also test for other functions //
//                                                                  //
//*******************************************************************/
    // test_HFL_init();
    // test_HFL_create_file();
    // test_HFL_open_file();
    // test_HFL_close_file();
    // test_get_mapArray();
    // test_HFL_insert_rec();
    // test_HFL_get_first_rec();
    
//***************************************************
// test big number of records here                  *
// insert 50000 records, close, reopen, get_next.   *
//***************************************************
    // test_HFL_get_next_rec();   

    // test_HFL_get_this_rec();
   
    // test_scanner();


/********************************************************************/
//                                                                  //
//  This is the given test function, also test for memory leaks     //
//                  Need to run this test with:                     //
//         valgrind --leak-check=full --show-leak-kinds=all \       //
//                       --track-origins=yes'                       //
//*******************************************************************/
    given_test_function();


    return 0;
}

// This is the given test function
void given_test_function()
{
    int errc = 0;

    HFL_init();

    errc = HFL_create_file("testing.dat");
    if (errc < 0) HFL_print_error(errc);

    fileDesc fd = HFL_open_file("testing.dat");

    // The number of records to insert into the file
    int numRecords = 1000;
    printf("*********  Insert 1000 records. **********\n");
    // Generate some nonsense records and insert them into the file
    //******************************************************************
    //                                                                 *
    //   I changed this generating function, the 2nd attribute of      *
    //  record is now 'i', equals its rid, such that it will be easier *
    //  to know the rid of a record.                                   *  
    //                                                                 *
    /******************************************************************/
    for (int i = 0; i < numRecords; i++) {
        record rec = mkRecord(i % 10, i, (i + 2) % 10,
                 'a', 'b', 'c', 'd');
        recordID rid = HFL_insert_rec(fd, &rec);
        printf("inserted rid = %d\t", rid);
    }
    printf("\n");

    record* recPtr = (record *)malloc(sizeof(record));
    // Retrieve the first record from the file    
    printf("******* retrive the first record from the file ********\n");
    pause();
    errc = HFL_get_first_rec(fd, &recPtr);
    if (errc < 0) HFL_print_error(errc);
    // Print attributes of the first record
    printf("the first record is :  ");
    print_rec(*recPtr);
    printf("\n");
    
    printf("******** Iterate through all records in the file *******\n");
    pause();
    // Iterate through all records in the file
    for (int i = 0; i < numRecords - 1; i++) {
        errc = HFL_get_next_rec(fd, &recPtr);
        if (errc < 0) HFL_print_error(errc);
        // Print attributes of the next record
        print_rec(*recPtr);
    }
    printf("\n");

    printf("******** Retrive the record with ID = 2 *********\n");
    pause();
    // Retrieve the record wth ID 2
    errc = HFL_get_this_rec(fd, 2, &recPtr);
    if (errc < 0) HFL_print_error(errc);
    // Print attributes of the record with ID 2
    print_rec(*recPtr);
    printf("\n");

    printf("******* Delete all records with even IDs between 0 and 100 *******\n");
    pause();
    // Delete all records with even IDs between 0 and 100
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            errc = HFL_delete_rec(fd, i);
            if (errc < 0) {
                HFL_print_error(errc);
            } else {
                numRecords--;
            }
        }
    }

    printf("**** Iterate a sceond time, to confirm that some have been deleted ****\n");
    pause();
    // Iterate through the records in the file a second time
    // to confirm that some have been deleted
    errc = HFL_get_first_rec(fd, &recPtr);
    if (errc < 0) HFL_print_error(errc);
    // Print attributes of the first record
    print_rec(*recPtr);
    printf("\n");

    for (int i = 0; i < numRecords - 1; i++) {
        errc = HFL_get_next_rec(fd, &recPtr);
        if (errc < 0) HFL_print_error(errc);
        // Print attributes of the next record
        print_rec(*recPtr);
    }
    printf("\n");

    printf("******* Try to retrive record with ID = 2, should be missing ********\n");
    pause();
    // Try to retrieve the record with ID 2--it should be missing
    errc = HFL_get_this_rec(fd, 2, &recPtr);
    assert (errc < 0); 
    HFL_print_error(errc);

    printf("***** Unpin all blocks in buffer pool, then close the file. *****\n");
    // Close the file, need to unpin first.
    show_bufferPool();
    pause();
    unpin_all();
    errc = HFL_close_file(fd);
    if (errc < 0) HFL_print_error(errc);

    printf("****** Memory leak test, free all mamory in heap ******\n");
    pause();
    free(recPtr);
    free_heap_memory();
}

void test_scanner() {
    int err = 0;
    HFL_init();
    HFL_create_file("test_file_name");
    int fd = HFL_open_file("test_file_name");
    // insert 500 records
    printf("insert 500 records\n");
    for (int i = 0; i < 500; ++i)
    {
        record rec = mkRecord(i, i+1, 2*i, 'a', 'b', 'c', 'd');
        HFL_insert_rec(fd, &rec);
    }
    record *r = (record *)malloc(sizeof(record));   

    printf("****** test different scanDesc ******\n");
    pause();
    int s1, s2;
    s1 = HFL_open_file_scan(fd);
    s2 = HFL_open_file_scan(fd);
    printf("s1 = %d, s2 = %d\n", s1, s2);
    printf("\nget 3 records from s1\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_find_next_rec(s1, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\nget 3 records from s2\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_find_next_rec(s2, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\nget 3 records from s1\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_find_next_rec(s1, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\nget 3 records from s2\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_find_next_rec(s2, &r);
        HFL_print_error(err);
        print_rec(*r);
    }

    printf("\n******* test with get_next / get_first ********\n");
    printf("\n**** get first ****\n");
    pause();
    err = HFL_get_first_rec(fd, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\n****** get next 3 times ******\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_get_next_rec(fd, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\n*****  get 3 using s1, should start by 6  *****\n");
    pause();
    for (int i = 0; i < 3; ++i)
    {
        err = HFL_find_next_rec(s1, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\n******* close s1 **********\n");
    pause();
    HFL_close_file_scan(s1);
    printf("******** see if s1 is closed, should print error message ********\n");
    err = HFL_find_next_rec(s1, &r);
    HFL_print_error(err);
    print_rec(*r);

    printf("\n*********** using new s3 scan all 500 records **********\n");
    pause();
    int s3 = HFL_open_file_scan(fd);
    for (int i = 0; i < 500; ++i)
    {
        err = HFL_find_next_rec(s3, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\n************ scan with deleted records *************\n");
    printf("delete 100 to 249, 300 to 450\n");
    pause();
    for (int i = 100; i < 250; ++i)
    {
        HFL_delete_rec(fd, i);
    }
    for (int i = 300; i < 451; ++i)
    {
        HFL_delete_rec(fd, i);
    }
    printf("*********************************************\n");
    printf("using new s1 scan 500 times ,should print error meesage\n");
    pause();
    s1 = HFL_open_file_scan(fd);
    for (int i = 0; i < 500; ++i)
    {
        err = HFL_find_next_rec(s1, &r);
        HFL_print_error(err);
        if (err != 0) {
            break;
        }
        print_rec(*r);
    }
}

void test_HFL_get_this_rec() {
    int err = 0;
    HFL_init();
    HFL_create_file("test_file_name");
    int fd = HFL_open_file("test_file_name");
    // insert 500 records
    printf("insert 500 records\n");
    for (int i = 0; i < 500; ++i)
    {
        record rec = mkRecord(i, i+1, 2*i, 'a', 'b', 'c', 'd');
        HFL_insert_rec(fd, &rec);
    }
    record *r = (record *)malloc(sizeof(record));    
    printf("\nget id = 0\n");
    err = HFL_get_this_rec(fd, 0, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 239\n");
    err = HFL_get_this_rec(fd, 239, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 240\n");
    err = HFL_get_this_rec(fd, 240, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 479\n");
    err = HFL_get_this_rec(fd, 479, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 480\n");
    err = HFL_get_this_rec(fd, 480, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 499\n");
    err = HFL_get_this_rec(fd, 499, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\nget id = 500 (doesn't exist)\n");
    err = HFL_get_this_rec(fd, 500, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\n*********** test get deleted record ***********\n");
    pause();
    HFL_delete_rec(fd, 50);
    err = HFL_get_this_rec(fd, 50, &r);
    HFL_print_error(err);
    print_rec(*r);
}

void pause() {
    printf("Press any key to continue ...\n");
    getchar();
    getchar();
    return;
}

void test_HFL_get_next_rec() {
    int err = 0;
    HFL_init();
    HFL_create_file("test_file_name");
    int fd = HFL_open_file("test_file_name");
    // insert 500 records
    printf("********* insert 500 records *********\n");
    pause();
    for (int i = 0; i < 500; ++i)
    {
        record rec = mkRecord(i, i+1, 2*i, 'a', 'b', 'c', 'd');
        HFL_insert_rec(fd, &rec);
    }
    show_bufferPool();
    show_fdMetaTable();
    printf("******* get 1st and next 100 *******\n");
    pause();
    record *r = (record *)malloc(sizeof(record));
    err = HFL_get_first_rec(fd, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\n");
    // keep get next 100
    for (int i = 0; i < 100; ++i)
    {
        err = HFL_get_next_rec(fd, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\n");
    show_bufferPool();
    // pause();
    printf("\nnow, currentRecID = %d, currentBlockID = %d\n", \
        fdMetaTable[fd - 3].currentRecID, fdMetaTable[fd - 3].currentID);
    printf("*****************************************************\n");
    printf("**************** delete 40, get next100 **************\n");
    // pause();
    for (int i = 100; i < 140; ++i)
    {
        HFL_delete_rec(fd, i + 1);
    }
    for (int i = 0; i < 100; ++i)
    {
        err = HFL_get_next_rec(fd, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\nnow, currentRecID = %d, currentBlockID = %d\n", \
        fdMetaTable[fd - 3].currentRecID, fdMetaTable[fd - 3].currentID);
    printf("*****************************************************\n");
    printf("**************** keep get next 259 **************\n");
    // pause();
    for (int i = 0; i < 259; ++i)
    {
        err = HFL_get_next_rec(fd, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\nnow, currentRecID = %d, currentBlockID = %d\n", \
        fdMetaTable[fd - 3].currentRecID, fdMetaTable[fd - 3].currentID);
    printf("*****************************************************\n");
    printf("************ keep get next 1 (doesn't exist) ***********\n");
    // pause();
    err = HFL_get_next_rec(fd, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\n");
    printf("********* add 50000 records, close the file, and reopen *********\n");
    pause();
    for (int i = 0; i < 50000; ++i)
    {
        unpin_all();
        record rec = mkRecord(i, i+1, 2*i, 'a', 'b', 'c', 'd');
        int id = HFL_insert_rec(fd, &rec);
        err = HFL_get_this_rec(fd, id, &r);
        print_rec(*r);
        printf("\tid = %d\t", id);
    }
    unpin_all();
    HFL_close_file(fd);
    printf("***** reopen file ******\n");
    fd = HFL_open_file("test_file_name");
    show_fdMetaTable();
    printf("************* get first **************\n");
    pause();
    err = HFL_get_first_rec(fd, &r);
    HFL_print_error(err);
    print_rec(*r);
    printf("\n");
    printf("*************** get next 50000 *************\n");
    pause();
    for (int i = 0; i < 50000; ++i)
    {
        unpin_all();
        err = HFL_get_next_rec(fd, &r);
        HFL_print_error(err);
        print_rec(*r);
    }
    printf("\n");
}

void test_HFL_get_first_rec() {
    int err = 0;
    HFL_init();
    HFL_create_file("test_file_name");
    int fd = HFL_open_file("test_file_name");
    // insert 500 records
    for (int i = 0; i < 470; ++i)
    {
        record rec = mkRecord(i, i+1, 2*i, 'a', 'b', 'c', 'd');
        HFL_insert_rec(fd, &rec);
    }
    record *r = (record *)malloc(sizeof(record));
    HFL_get_first_rec(fd, &r);
    print_rec(*r);
    printf("\n***************  Delete 100 record  **************\n");
    for (int i = 0; i < 100; ++i)
    {
        HFL_delete_rec(fd, i);
    }
    HFL_get_first_rec(fd, &r);
    print_rec(*r);
    printf("\n***************  Delete 200 record  **************\n");
    for (int i = 100; i < 300; ++i)
    {
        HFL_delete_rec(fd, i);
    }
    HFL_get_first_rec(fd, &r);
    print_rec(*r);
    printf("\n***************  Delete 200 record  **************\n");
    printf("\n********** Sould print ERROR **********\n");
    for (int i = 300; i < 500; ++i)
    {
        HFL_delete_rec(fd, i);
    }
    err = HFL_get_first_rec(fd, &r);
    if (err != 0) {
        HFL_print_error(err);
    }

}

void print_rec(record rec) {
    printf("%d  %d  %d  %c  %c  %c, \t", rec.attribute1, rec.attribute2, \
        rec.attribute3, rec.attribute4, rec.attribute5, rec.attribute6);
}

void unpin_all() {
    int err = 0;
    for (int i = 0; i < BUFFERSIZE; ++i)
    {
        if (bufferPool[i].pinCount > 0) {
            err = BM_unpin_block(&bufferPool[i]);
            BM_print_error(err);
        }
    }
}

void test_HFL_insert_rec() {
    HFL_init();
    int err = HFL_create_file("test_file_name");
    HFL_print_error(err);
    int fd = HFL_open_file("test_file_name");
    for (int i = 0; i < 450; ++i)
    {
        record r = mkRecord(i, i+1, i*2, 'a', 'b', 'c', 'd');
        recordID rid = HFL_insert_rec(fd, &r);
        printf("%d  ", rid);
    }
    printf("\n");
    char *data = bufferPool[1].data;
    for (int i = 0; i < FRAMESIZE; ++i)
    {
         printf("%d ", data[i]);
    }
    printf("\nblock->freeSpace = %d\n", bufferPool[1].freeSpace);

///////////////////////////////////////////////////////////////////
    printf("***************************************************\n");
    printf("simulate the situation that we deleted former records\n");
    for (int i = 0; i < 100; ++i){
        HFL_delete_rec(3, i);
    }
    printf("\nblock->freeSpace = %d\n", bufferPool[0].freeSpace);
    for (int i = 0; i < 300; ++i)
    {
        record r = mkRecord(i, i+1, i*2, 'a', 'b', 'c', 'd');
        recordID rid = HFL_insert_rec(fd, &r);
        printf("%d  ", rid);
    } 
    printf("\nblock->freeSpace = %d\n", bufferPool[2].freeSpace);

    show_bufferPool();
    unpin_all();
    HFL_close_file(3);

    HFL_open_file("test_file_name");
    block *blockPtr = NULL;
    BM_get_first_block(3, &blockPtr);
    printf("reopen, freeSpace 0 = %d\n", blockPtr->freeSpace);
    BM_get_next_block(3, &blockPtr);
    printf("reopen, freeSpace 1 = %d\n", blockPtr->freeSpace);
    BM_get_next_block(3, &blockPtr);
    printf("reopen, freeSpace 2 = %d\n", blockPtr->freeSpace);
}

void test_get_mapArray() {
    BM_init();
    BM_create_file("test");
    BM_open_file("test");
    int id;
    BM_alloc_block(3, &id);
    block *b1;
    BM_get_first_block(3, &b1);
    char *map = (char *)malloc(pageCapacity * sizeof(char));
    memset(map, 0, pageCapacity);
    get_mapArray(&map, b1);
    for (int i = 0; i < pageCapacity; ++i)
    {
        fprintf(stderr, "%d  ", map[i]);
    }

    printf("\n%lu   pageCapacity: %d\n", sizeof(map), pageCapacity);

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
            printf("currentID: %d, fp: %p, currentRecID: %d\n", \
                fdMetaTable[i].currentID, fdMetaTable[i].fp, \
                fdMetaTable[i].currentRecID);
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



void test_HFL_close_file() {
    printf(" should be no error message.\n");
    int err = 0;
    HFL_init();
    err = HFL_create_file("test_file_name");
    if (err != 0) {
        HFL_print_error(err);
    }
    int fd = HFL_open_file("test_file_name");
    err = HFL_close_file(fd);
    HFL_print_error(err);
}

void test_HFL_open_file() {
    printf("fd should be 3\n");
    int err = 0;
    HFL_init();
    err = HFL_create_file("test_file_name");
    if (err != 0) {
        HFL_print_error(err);
    }
    int fd = HFL_open_file("test_file_name");
    printf("fd = %d\n", fd);
}

void test_HFL_create_file() {
    int err = 0;
    HFL_init();
    err = HFL_create_file("test_file_name");
    if (err != 0) {
        HFL_print_error(err);
    }
    printf("Should create file in ./data\n");
}

void test_HFL_init() {
    printf("--------- should print 100 -1's ---------\n");
    HFL_init();
    for (int i = 0; i < MAXSDNUM; ++i)
    {
        printf("%d  ", scannerTable[i].currentRid);
    }
}

/* Utility function for creating a record struct */
record mkRecord(int attr1, int attr2, int attr3,
                char attr4, char attr5, char attr6, char attr7)
{
    record r;
    r.attribute1 = attr1;
    r.attribute2 = attr2;
    r.attribute3 = attr3;
    r.attribute4 = attr4;
    r.attribute5 = attr5;
    r.attribute6 = attr6;
    r.attribute7 = attr7;
    return r;
}
