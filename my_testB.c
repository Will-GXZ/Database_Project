#include <assert.h>
#include <string.h>
#include "projectA.h"
#include "projectB.h"



record mkRecord(int attr1, int attr2, int attr3,
        char attr4, char attr5, char attr6, char attr7);

void test_HFL_init(void);
void test_HFL_create_file(void);
void test_HFL_open_file(void);
void test_HFL_close_file(void);
void test_my_queue(void);
void test_get_mapArray(void);
void show_fdMetaTable(void);
void show_bufferPool(void);
void test_HFL_insert_rec(void);


int main(int argc, char const *argv[]) {
    int err = 0;
    
    // test_HFL_init()
    // test_HFL_create_file();
    // test_HFL_open_file();
    // test_HFL_close_file();
    // test_my_queue();
    // test_get_mapArray();
    test_HFL_insert_rec();

    return 0;
}

void test_HFL_insert_rec() {
    int err = 0;
    HFL_init();
    err = HFL_create_file("test_file_name");
    int fd = HFL_open_file("test_file_name");
    for (int i = 0; i < 1; ++i)
    {
        record r = mkRecord(i, i+1, i*2, 'a', 'b', 'c', 'd');
        recordID rid = HFL_insert_rec(fd, &r);
        printf("%d  ", rid);
    }

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

void test_my_queue() {
    Q_init();
    for (int i = 0; i < 200; ++i)
    {
        Q_enqueue(i);
    }
    for (int i = 0; i < 100; ++i)
    {
        int t = Q_dequeue();
        printf("%d  ", t);
    }
    printf("\n****** still have 100 elements, insert 100 more ******\n");
    for (int i = 0; i < 200; ++i)
    {
        Q_enqueue(i);
    }
    printf("\ndequeue all elements, should be from 100 to 199, and 0 to 99\n");
    for (int i = 0; i < 200; ++i)
    {
        if (! Q_isempty()) {
            int t = Q_dequeue();
            printf("%d  ", t);
        }
    }
    Q_free();  
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
        printf("%d  ", scannerTable[i].currentBlockID);
    }
}

// int main(int argc, char const *argv[])
// {

//     int errc = 0;

//     HFL_init();

//     errc = HFL_create_file("testing.dat");
//     if (errc < 0) HFL_print_error(errc);

//     fileDesc fd = HFL_open_file("testing.dat");

//     // The number of records to insert into the file
//     int numRecords = 1000;

//     // Generate some nonsense records and insert them into the file
//     for (int i = 0; i < numRecords; i++) {
//         record rec = mkRecord(i % 10, (i + 1) % 10, (i + 2) % 10,
//                  'a', 'b', 'c', 'd');
//         recordID rid = HFL_insert_rec(fd, &rec);
//     }

//     record* recPtr = NULL;
    
//     // Retrieve the first record from the file    
//     errc = HFL_get_first_rec(fd, &recPtr);
//     if (errc < 0) HFL_print_error(errc);
//     // Print attributes of the first record

//     // Iterate through all records in the file
//     for (int i = 0; i < numRecords - 1; i++) {
//         errc = HFL_get_next_rec(fd, &recPtr);
//         if (errc < 0) HFL_print_error(errc);
//         // Print attributes of the next record
//     }

//     // Retrieve the record wth ID 2
//     errc = HFL_get_this_rec(fd, 2, &recPtr);
//     if (errc < 0) HFL_print_error(errc);
//     // Print attributes of the record with ID 2

//     // Delete all records with even IDs between 0 and 100
//     for (int i = 0; i < 100; i++) {
//         if (i % 2 == 0) {
//         errc = HFL_delete_rec(fd, i);
//         if (errc < 0) {
//             HFL_print_error(errc);
//         } else {
//             numRecords--;
//         }
//     }
//     }

//     // Iterate through the records in the file a second time
//     // to confirm that some have been deleted
//     errc = HFL_get_first_rec(fd, &recPtr);
//     if (errc < 0) HFL_print_error(errc);
//     // Print attributes of the first record
//     for (int i = 0; i < numRecords - 1; i++) {
//         errc = HFL_get_next_rec(fd, &recPtr);
//         if (errc < 0) HFL_print_error(errc);
//         // Print attributes of the next record
//     }

//     // Try to retrieve the record with ID 2--it should be missing
//     errc = HFL_get_this_rec(fd, 2, &recPtr);
//     assert (errc < 0);

//     // Close the file
//     errc = HFL_close_file(fd);
//     if (errc < 0) HFL_print_error(errc);

//     return 0;
// }

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
