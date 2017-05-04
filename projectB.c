#include "projectA.h"
#include "projectB.h"


#define DEBUG

scanner *scannerTable = NULL;

void HFL_init() 
{
    // initialize scannerTable first.
    scannerTable = (scanner *)malloc(100 * sizeof(scanner));
    if (scannerTable == NULL) {
        fprintf(stderr, "**** HFL_init failed. ****\n");
        exit(1);
    }
    for (int i = 0; i < 100; ++i)
    {
        scannerTable[i].fd = -1;
        scannerTable[i].currentBlockID = -1;
        scannerTable[i].currentOffset = -1;
    }
    // after, initialize buffer manager
    BM_init();

    // initialize the deleted recID queue
    Q_init();

    #ifdef DEBUG
        printf("******* HFL_init ********\n");
    #endif
}


errCode HFL_create_file(char* filename)
{   
    int err;
    err = BM_create_file(filename);
    if (err != 0) {
        BM_print_error(err);
        return 2;
    }

    #ifdef DEBUG
        fprintf(stderr, "********** HFL_create_file: %s *********\n", filename);
    #endif
    return 0;
}


fileDesc HFL_open_file(char* filename)
{
    int fd = BM_open_file(filename);

    #ifdef DEBUG
        fprintf(stderr, "******** HFL_open_file: %s ********\n", filename);
    #endif
    return fd;
}


errCode HFL_close_file(fileDesc fd)
{   
    int err = 0;
    // need to check if all scanners of this fd have been closed
    for (int i = 0; i < MAXSDNUM ; ++i)
    {
        if (scannerTable[i].fd == fd) { return 4; } 
    }

    err = BM_close_file(fd);
    if (err != 0) {
        BM_print_error(err);
        return 4;
    }

    #ifdef DEBUG
        fprintf(stderr, "********* HFL_close_file: %d *********\n", fd);
    #endif
    return 0;
}

void get_mapArray(char ** map, const block *blockPtr) {
    memcpy(*map, blockPtr->data + ENTRYLENGTH * pageCapacity, pageCapacity);
}

// When delete a record, put recID in a queue, when we insert a record, dequeue
// a empty recordID, if queue is empty, add new record in last block.
recordID HFL_insert_rec(fileDesc fd, record* rec)
{
    int err = 0;
    int targetID = -1;
    // first, check if there is deleted recID
    if (! Q_isempty()) {
        targetID = Q_dequeue(); // insert new record to this recID
        int pageID = targetID / pageCapacity;
        int offset = targetID % pageCapacity;
        // get target page from BM
        block *targetPage = NULL;
        err = BM_get_this_block(fd, pageID, &targetPage);
        if (err != 0) {
            BM_print_error(err);
            return -1;
        }
        // now we got target page : targetPage pointer
        if (targetPage->freeSpace == 0) { return -1; }// something wrong with queue 
        char *map = (char *)malloc(pageCapacity * (sizeof(char)));
        get_mapArray(&map, targetPage);
        if (map[offset] == 1) { 
            return -1; 
        } else {
            map[offset] = 1;
            memcpy(targetPage->data + pageCapacity * ENTRYLENGTH , map, \
                pageCapacity); // update map 
        }
        // do insertion
        memcpy(targetPage->data + offset * ENTRYLENGTH, rec, sizeof(rec));
        
        targetPage->freeSpace --;
    } else {
        // if the queue is empty, see if last block has empty slot
        int pageID = fdMetaTable[fd - 3].lastBlockID;
        block *targetPage = NULL;
        err = BM_get_this_block(fd, pageID, &targetPage);
        if (err != 0) {
            BM_print_error(err);
            return -1;
        }
        if (targetPage->freeSpace != 0) {
        // insert here
            // find empty slot (offset)
            int offset = -1;
            char *map = (char *)malloc(pageCapacity * (sizeof(char)));
            get_mapArray(&map, targetPage);
            for (int i = 0; i < pageCapacity; ++i)
            {
                if (map[i] == -1) { 
                    offset = i;
                    map[i] = 1;
                    break; 
                }
            }
            // update map
            memcpy(targetPage->data + pageCapacity * ENTRYLENGTH , map, \
                pageCapacity);
            // do insertion
            memcpy(targetPage->data + offset * ENTRYLENGTH, rec, sizeof(rec));
        
            targetPage->freeSpace --;
            targetID = pageID * pageCapacity + offset;
        } else {
            // allock new block, insert new record at first slot, and put all
            // rest recIDs of this new block in queue
            int pageID = -1;
            err = BM_alloc_block(fd, &pageID);
            if (err != 0) {
                BM_print_error(err);
                return -1;
            }
            targetID = pageID * pageCapacity + 0; // offset = 0
            // insert at the first slot of this new page 
            err = BM_get_this_block(fd, pageID, &targetPage);
            if (err != 0) {
                BM_print_error(err);
                return -1;
            }
            char *map = (char *)malloc(pageCapacity * (sizeof(char)));
            get_mapArray(&map, targetPage);
            map[0] = 1;
            targetPage->freeSpace --;
            // update map
            memcpy(targetPage->data + pageCapacity * ENTRYLENGTH , map, \
                pageCapacity);
            // do insertion
            memcpy(targetPage->data + 0 * ENTRYLENGTH, rec, sizeof(rec));

            // put the rest recordIDs in queue
            for (int i = 1; i < pageCapacity; ++i)
            {
                int ID = pageID * pageCapacity + i;
                Q_enqueue(ID);
            }
        }
    }

    #ifdef DEBUG
        fprintf(stderr, "******** HFL_insert_rec: recID = %d ********\n", targetID);
    #endif

    return targetID;
}

errCode HFL_delete_rec(fileDesc fd, recordID rid)
{
    fprintf(stderr, "Attempting to delete record with ID %d from file %d\n", rid, fd);
    return 0;
}


errCode HFL_get_first_rec(fileDesc fd, record** rec)
{   


    #ifdef DEBUG
        fprintf(stderr, "******** HFL_get_first_rec: fd = %d ********\n", fd);
    #endif
    return 0;
}

errCode HFL_get_next_rec(fileDesc fd, record** rec)
{
    fprintf(stderr, "Attempting to get next record from file %d\n", fd);
    return 0;
}

errCode HFL_get_this_rec(fileDesc fd, recordID rid, record** rec)
{
    fprintf(stderr, "Attempting to get record %d from file %d\n", rid, fd);
    return 0;
}

scanDesc HFL_open_file_scan(fileDesc fd)
{
    fprintf(stderr, "Opening a file scan for file %d\n", fd);
    return 0;
}

errCode HFL_find_next_rec(scanDesc sd, record** rec)
{
    fprintf(stderr, "scanDesc %d is scanning to the next record\n", sd);
    return 0;
}

errCode HFL_close_file_scan(scanDesc sd)
{
    fprintf(stderr, "Closing file scan %d", sd);
    return 0;
}

void HFL_print_error(errCode ec)
{
    if (ec == 0) { return; }
    fprintf(stderr, "|-----------------------------------------|\n");
    fprintf(stderr, "|             ERROR MESSAGES              |\n");
    if (ec == 2) {
        fprintf(stderr, "|          ERROR: HFL_create_file         |\n");
    }
    if (ec == 3) {
        fprintf(stderr, "|           ERROR: HFL_open_file          |\n");
    }
    if (ec == 4) {
        fprintf(stderr, "|          ERROR: HFL_close_file          |\n");
    }
    if (ec == 5) {  
        fprintf(stderr, "|          ERROR: HFL_insert_rec          | \n");
    }
    if (ec == 6) {
        fprintf(stderr, "|          ERROR: HFL_delete_rec          |\n");
    }   
    if (ec == 7) {
        fprintf(stderr, "|        ERROR: HFL_get_first_rec         |\n");
    }
    if (ec == 8) {
        fprintf(stderr, "|         ERROR: HFL_get_next_rec         |\n");
    }
    if (ec == 9) {
        fprintf(stderr, "|        ERROR: HFL_get_this_rec          |\n");
    }
    if (ec == 10) {
        fprintf(stderr, "|       ERROR: HFL_open_file_scan         |\n");
    }
    if (ec == 11) {
        fprintf(stderr, "|        ERROR: HFL_find_next_rec         |\n");
    }
    if (ec == 12) {
        fprintf(stderr, "|       ERROR: HFL_close_file_scan        |\n");
    }
    fprintf(stderr, "|_________________________________________|\n");
}












