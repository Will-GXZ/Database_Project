#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        scannerTable[i].currentRid = -1;
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
        return -2;
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
        if (scannerTable[i].fd == fd) { return -4; } 
    }

    err = BM_close_file(fd);
    if (err != 0) {
        BM_print_error(err);
        return -4;
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
// a empty recordID, if queue is empty, alloc new block, add all new empty slot(rid)
// in queue.
recordID HFL_insert_rec(fileDesc fd, record* rec)
{
    int err = 0;
    int targetID = -1;
    block *targetPage = NULL;
    // first, check if there is deleted recID
    if (! Q_isempty()) {
        targetID = Q_dequeue(); // insert new record to this recID
        int pageID = targetID / pageCapacity;
        int offset = targetID % pageCapacity;
        // get target page from 
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
        *((int *)(targetPage->data + (ENTRYLENGTH + 1) * pageCapacity)) -= 1;
        // do insertion
        memcpy(targetPage->data + offset * ENTRYLENGTH, rec, sizeof(record));
        
        targetPage->freeSpace --;
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
        targetPage->freeSpace --;
        // update map and data freeSpace
        map[0] = 1;
        memcpy(targetPage->data + pageCapacity * ENTRYLENGTH , map, \
            pageCapacity);
        *((int *)(targetPage->data + (ENTRYLENGTH + 1) * pageCapacity)) -= 1;
        // do insertion
        memcpy(targetPage->data + 0 * ENTRYLENGTH, rec, sizeof(record));

        // put the rest recordIDs in queue
        for (int i = 1; i < pageCapacity; ++i)
        {
            int ID = pageID * pageCapacity + i;
            Q_enqueue(ID);
        }
    }

    #ifdef DEBUG
        fprintf(stderr, "******** HFL_insert_rec: recID = %d ********\n", targetID);
    #endif

    return targetID;
}

// calculate pageID and offset of input rid, and delete it, put deleted rid in queue
errCode HFL_delete_rec(fileDesc fd, recordID rid)
{
    // get page first
    int err = 0;
    int pageID = rid / pageCapacity;
    int offset = rid % pageCapacity;
    block *blockPtr = NULL;
    err = BM_get_this_block(fd, pageID, &blockPtr);
    if (err != 0) {
        BM_print_error(err);
        return -6;
    }
    // got page: blockPtr->, delete.
    char *map = (char *)malloc(pageCapacity * (sizeof(char)));
    get_mapArray(&map, blockPtr);   
    if (map[offset] != 1) {
        fprintf(stderr, "ERROR: Record with fd= %d, rid = %d doesn't exist.", \
            fd, rid);
        return 0;
    }
    // update freeSpace information and map
    map[offset] = -1;
    memcpy(blockPtr->data + pageCapacity * ENTRYLENGTH , map, \
    pageCapacity); // update map 
    *((int *)(blockPtr->data + (ENTRYLENGTH + 1) * pageCapacity)) -= 1;
    blockPtr->freeSpace ++;

    // enqueue deleted rid
    Q_enqueue(rid);

    #ifdef DEBUG
        fprintf(stderr, "*********** HFL_delete_rec,  ID = %d ,fd = %d **********\n",\
             rid, fd);
    #endif
    return 0;
}


// check first block first, if there is no block in first block, then loop to 
// check next block. Update currentRecID
errCode HFL_get_first_rec(fileDesc fd, record** rec)
{   
    int err = 0;
    block *blockPtr = NULL;
    // check first block first
    err = BM_get_first_block(fd, &blockPtr);
    if (err != 0) {
        BM_print_error(err);
        return -7;
    }
    if (blockPtr->freeSpace == pageCapacity) {
        // loop to find next block
        int i = 1;
        while (i < fdMetaTable[fd - 3].blockNumber) {
            err = BM_get_next_block(fd, &blockPtr);
            if (err != 0) {
                BM_print_error(err);
                return -7;
            }
            if (blockPtr->freeSpace != pageCapacity)
            {
                break;
            }
            i ++;
        }
    }
    if (blockPtr->freeSpace == pageCapacity) {
        printf("ERROR: HFL_get_first_rec, there is no record in this file.\n");
        return -7;
    }
    // now, we got blockPtr contains first record, loop to find offset of record
    int offset = -1;
    char *map = (char *)malloc(pageCapacity * (sizeof(char)));
    get_mapArray(&map, blockPtr);
    for (int i = 0; i < pageCapacity; ++i)
    {
        if (map[i] == 1) { 
            offset = i;
            break;
        }
    }
    // read record
    memcpy(*rec, blockPtr->data + offset * ENTRYLENGTH, sizeof(record));

    // update currentRecID in metadata
    int rid = offset + pageCapacity * blockPtr->blockID;
    fdMetaTable[fd - 3].currentRecID = rid;

    #ifdef DEBUG
        fprintf(stderr, "******** HFL_get_first_rec: fd = %d, rid = %d ********\n",\
             fd, rid);
    #endif
    return 0;
}

// check if currentBlock has next record, if not, loop to find next record
errCode HFL_get_next_rec(fileDesc fd, record** rec)
{
    int err = 0;
    int currentBlockID = fdMetaTable[fd - 3].currentID;
    int currentOffset = fdMetaTable[fd - 3].currentRecID % pageCapacity;
    block *blockPtr = NULL;
    err = BM_get_this_block(fd, currentBlockID, &blockPtr);
    if (err != 0) {
        BM_print_error(err);
        return -8;
    }
    // check if current block has next record
    char *map = (char *)malloc(pageCapacity * (sizeof(char)));
    get_mapArray(&map, blockPtr);
    int offset = -1;
    for (int i = currentOffset + 1; i < pageCapacity; ++i)
    {
        if (map[i] == 1) {
            offset = i;
            break;
        }
    }
    // if current page doesn't have next record, loop.
    if (offset == -1) {
        int i = currentBlockID;
        // find block first.
        while (i < fdMetaTable[fd - 3].blockNumber) {
            err = BM_get_next_block(fd, &blockPtr);
            if (err != 0) {
                BM_print_error(err);
                printf("ERROR: HFL_get_next_rec, This is already the last record.\n");
                return -8;
            }
            if (blockPtr->freeSpace != pageCapacity) {
                // get it.
                break;
            }
            i = blockPtr->blockID;
        }
        // see if we have found proper block
        if (i == fdMetaTable[fd - 3].blockNumber) {
            printf("ERROR: HFL_get_next_rec, cannot find next rec.\n");
            return -8;
        }
        // if we have found block, we then find offset.
        char *map = (char *)malloc(pageCapacity * (sizeof(char)));
        get_mapArray(&map, blockPtr);
        for (int i = 0; i < pageCapacity; ++i)
        {
            if (map[i] == 1) {
                offset = i;
                break;
            }
        }
        if (offset == -1) {
            printf("ERROR: HFL_get_next_rec, cannot find next rec.\n");
            return -8;
        }
    }
    // now we got blockPtr and offset
    // read record
    memcpy(*rec, blockPtr->data + offset * ENTRYLENGTH, sizeof(record));
    // update current RecID;
    int rid = blockPtr->blockID * pageCapacity + offset;
    fdMetaTable[fd - 3].currentRecID = rid;

    #ifdef DEBUG
        fprintf(stderr, "********* HFL_get_next_rec, fd = %d, rid = %d *********\n",\
         fd, rid);
    #endif
    return 0;
}

// calculate pageID and offset from rid first, than find record.
errCode HFL_get_this_rec(fileDesc fd, recordID rid, record** rec)
{
    int err = 0;
    int pageID = rid / pageCapacity;
    int offset = rid % pageCapacity;

    if (rid < 0 || pageID >= fdMetaTable[fd - 3].blockNumber) {
        printf("ERROR: HFL_get_this_rec, recordID = %d doesn't exist;\n", rid);
        return -9;
    }

    block *blockPtr = NULL;
    // find page first
    err = BM_get_this_block(fd, pageID, &blockPtr);
    if (err != 0) {
        BM_print_error(err);
        return -9;
    }
    // read record
    char *map = (char *)malloc(pageCapacity * (sizeof(char)));
    get_mapArray(&map, blockPtr);
    if (map[offset] == -1) {
        printf("ERROR: HFL_get_this_rec, recordID = %d doesn't exist;\n", rid);
        return -9;
    }
    memcpy(*rec, blockPtr->data + offset * ENTRYLENGTH, sizeof(record));

    #ifdef DEBUG
        fprintf(stderr, "****** HFL_get_this_rec: ID: %d from fd: %d ******\n", \
            rid, fd);
    #endif
    return 0;
}

// get a scanDesc from scannerTable, initialize the scanner.
scanDesc HFL_open_file_scan(fileDesc fd)
{
    int sd = -1;
    for (int i = 0; i < MAXSDNUM; ++i)
    {
        if (scannerTable[i].fd == -1) {
            sd = i;
            scannerTable[i].fd = fd;
            scannerTable[i].currentRid = -1;
            break;
        }
    }

    #ifdef DEBUG
        fprintf(stderr, "***** HFL_open_file_scan, fd = %d, sd = %d\n", fd, sd);
    #endif
    return sd;
}

errCode HFL_find_next_rec(scanDesc sd, record** rec)
{
    int err = 0;
    // find scan metadata first
    int rid = -1;
    int newRid = -1;
    int fd = -1;
    if (scannerTable[sd].fd < 3) {
        printf("ERROR: HFL_find_next_rec, sd is not exist\n");
        return -11;
    }
    fd = scannerTable[sd].fd;
    rid = scannerTable[sd].currentRid;

    int currentIDBackup = fdMetaTable[fd - 3].currentID;
    int currentRecIDBackup = fdMetaTable[fd - 3].currentRecID;
    if (rid == -1) // need to get first record
    {
        fdMetaTable[fd - 3].currentID = -1;
        fdMetaTable[fd - 3].currentRecID = -1;
        err = HFL_get_first_rec(fd, rec);
        if (err != 0)
        {
            HFL_print_error(err);
            return -11;
        }
        newRid = fdMetaTable[fd - 3].currentRecID;
        fdMetaTable[fd - 3].currentID = currentIDBackup;
        fdMetaTable[fd - 3].currentRecID = currentRecIDBackup;
    } else {
        // need to get next
        int pageID = rid / pageCapacity;
        fdMetaTable[fd - 3].currentID = pageID;
        fdMetaTable[fd - 3].currentRecID = rid;
        err = HFL_get_next_rec(fd, rec);
        if (err != 0) {
            HFL_print_error(err);
            return -11;
        }
        newRid = fdMetaTable[fd - 3].currentRecID;
        fdMetaTable[fd - 3].currentID = currentIDBackup;
        fdMetaTable[fd - 3].currentRecID = currentRecIDBackup;
    }
    scannerTable[sd].currentRid = newRid;

    #ifdef DEBUG
        fprintf(stderr, "*** scanDesc = %d is scanning to the next record ***\n", sd);
    #endif
    return 0;
}

errCode HFL_close_file_scan(scanDesc sd)
{
    if (scannerTable[sd].fd == -1) {
        printf("ERROR: HFL_close_file_scan, sd doesn't exist.\n");
        return -12;
    }
    scannerTable[sd].fd = -1;
    scannerTable[sd].currentRid = -1;

    #ifdef DEBUG
        fprintf(stderr, "***** Closing file scan %d ******", sd);
    #endif
    return 0;
}

void HFL_print_error(errCode ec)
{
    if (ec == 0) { return; }
    fprintf(stderr, "|-----------------------------------------|\n");
    fprintf(stderr, "|             ERROR MESSAGES              |\n");
    if (ec == -2) {
        fprintf(stderr, "|          ERROR: HFL_create_file         |\n");
    }
    if (ec == -4) {
        fprintf(stderr, "|          ERROR: HFL_close_file          |\n");
    }
    if (ec == -6) {
        fprintf(stderr, "|          ERROR: HFL_delete_rec          |\n");
    }   
    if (ec == -7) {
        fprintf(stderr, "|        ERROR: HFL_get_first_rec         |\n");
    }
    if (ec == -8) {
        fprintf(stderr, "|         ERROR: HFL_get_next_rec         |\n");
    }
    if (ec == -9) {
        fprintf(stderr, "|        ERROR: HFL_get_this_rec          |\n");
    }
    if (ec == -11) {
        fprintf(stderr, "|        ERROR: HFL_find_next_rec         |\n");
    }
    if (ec == -12) {
        fprintf(stderr, "|       ERROR: HFL_close_file_scan        |\n");
    }
    fprintf(stderr, "|_________________________________________|\n");
}












