# CS115 Database Systems Analysis Class Project

#### Xiaozheng Guo

Language: C

Created at Mar 31, 2017
____________________________

### Acknowledgements
I would like to express my very great appreciation to Prof.Athanassoulis for his valuable and constructive suggestions during the planning and development of this work. I would also like to extend my thanks to TAs of this course for their continuous and patient response in [piazza](https://piazza.com). 

I also wish to acknowledge the help provided by [tutorialspoint](https://www.tutorialspoint.com/c_standard_library) and [stackoverflow](https://stackoverflow.com/). To work on this project, I need to review and learn a lot of stuff in C and linux, therefore I search the Internet and these two web sites gave me very useful informations. 

______________________________

## Part A: The buffer manager
### Files in This Part
* projectA.h
    The given header file, contains APIs of this part. All I did in this part is to implement these APIs. I also put helper function declarations in this file to avoid *warning* when testing these functions. In this file, there are also variables' declarations like `block` structure and `metadata` structure.  
* projectA.c
    Implementations of all functions are in this file. For the details of each function please go over it, it's well commented.

*  my_test.c
    This file contains all test functions, including all unit test and memory test function. I put the given test function in this file as a `given_test_main()` function. 

*  TestMakefile
    This file is the *Makefile* that compile this part with *my_test.c* to for testing purpose.

*  Makefile
    This is the default *Makefile*, it is used to compile the whole project including **part A** and **part B** . For this **part A**, we can compile test using `make test`, this command will automatically run **TestMakefile**.

*  README.md
    This is this file itself, the contents of this file are as you see now.

*  .gitignore
    This is github default C ignore file.

### How to Compile 
Before you compile and run this part, you need to edit  **my_testA.c** to select which test function you want to run.  Because now I have commented out all code in the **main** function.

To compile this part for testing purpose, make sure you have all the files in the same directory, and type in `make reset` and `make testA`. 

### Details
#### Buffer Management
   Buffer Pool Structure
   This program use a dynamic array as data structure, each slot of the array stores a block structure.
     
   The definition of the array and block: 
    
```
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
    block *bufferPool = (block *)malloc(BUFFERSIZE * sizeof(block));
     
```
#### Buffer Pool Replacement Policy
   In my project, I choose to use a variant of LRU, called **clock** replacement, which is mentioned in text book. The implementation of this replacement policy is in `buffer_add_block` function in *projectA.c* file. I also write a well commented **test function** in *my_test.c* to test my implementation.
   
####  File Management
   1.  File Metadata
   In my work, I define a metadata structure to store information of each opened file. And I use a direct address table to store all metadata structures. In the meantime, maintain a metadata page for each file in disk. When we open a file, read in its metadata page, store metadata in memory. When we close a file, write back metadata accordingly.
   
     This is my definition of metadata structure:
     
   ```
    typedef struct _metadata {
        int currentID; // the blockID that is current in use
        int firstBlockID;
        int lastBlockID;
        int blockNumber; // the number of total page the file has
        int headerNumber; // the number of header pages of the file.
        int currentRecID; // (added when implement partB)
        char *fileName;
        FILE *fp;
    } metadata;
    max_fd = rlim.rlim_cur - 3; //because fd = 0, 1, 2 don't count.
    fdMetaTable = (metadata *)malloc(max_fd * sizeof(metadata));
  ```

   2.  File Organization
   In my implementation, I choose to use the **page directory** strategy to implement **heap file** organization. Specifically, the program  will store each page as a small file of 4096 bytes in disk, and for each database file,  maintain a series of header page as data pages' directory. Also, as I mentioned above, the program will maintain a metadata page for each database file in disk.

   Detailed roganization:
    
   *   Header Page

  ![Header page organization diagram](https://dl.dropboxusercontent.com/s/2xztywx1dmrqjtr/comp115proj_header.jpg?dl=0 )

   *   Data page

  ![Data page organization diagram](https://dl.dropboxusercontent.com/s/xcx9apto2sd70xg/comp115proj_page.jpg?dl=0)
 
   *   Metadata page

  ![Metadata page orgnization diagram](https://dl.dropboxusercontent.com/s/ahw67mb6mo9jbva/comp115proj_meta.jpg?dl=0)
  
   3. Naming
   * Header pages   : (filename)_h(headerID).head
   * Data pages     : (filename)_(blockID).dat
   * Metadata pages : (filename)_.meta

### Challenges and Things Worth Discussing 
  Apparently, it is not easy to implement these APIs. The first problem I met is how to make those decisions and design the whole program. I spent 4 days to design the whole system, including my file organization, buffer replacement strategy and helper functions. I also spent a lot of time to write pseudocode for each function. All of these efforts turn out to be very helpful in my formally implementation.

Secondly, because we are using C, we need to be extremely careful about **pointers and alloc-free** operations. If you try to free a block of memory that is already been freed, you will encounter double free. If you try to modify a block of memory that we do not own, you will have segment fault. If you fail to calculate the right offset or length of an array, you might also get a segmentation fault. In my implementation, I carefully check each of these situations and make sure they work well. These work takes me a lot of time to write **test case for each function**.

It might also be challenging for me that I am not familiar enough with C and Linux. During my working, I met a lot of problems about these two things and I had to search in Google or stackoverflow to find solutions. Like, we have to use **std = gnu99** not **std=c99** in Makefile to avoid warning: implicit declaration of function ‘fileno’. Also, we need to store the FILE pointer returned by fopen() in BM_open_file() function, such that we can do fclose() when we close that file in BM_close_file(). If we don’t do so, that FILE pointer will never be freed. That is, if we keep use BM_open_file() and BM_close_file() on different files, we will finally **use up all fds** and we cannot open other files any more.

Lastly, I think it is very interesting and useful to use **errCode**. In my implementation, if a outer function calls a middle function, and the middle function calls a inner function. If there is something wrong with the inner function, it will first print out a Error Message, and return its errCode. Then, the middle function get the errCode returned by the inner function, it will print that errCode and return its own errCode. Finally,  the outer function will get middle function’s errCode, it will also print out that errCode. Such that, we will get a **stack of Error Message**. With this stack, we can know where the error was generated and how functions interact with each other. It is very helpful when we need to debug.

### Test Cases
To implement these APIs, I need to write Unit Test for each function. In each of these test functions, I intentionally test boundary cases. Like, open a file with a fileName that doesn’t exist, try to add a new block into buffer pool when all blocks in buffer are pinned, close a file with a nonexistent file descriptor, get next block when current blockID is the last one, try to dispose a block when it is still pinned, etc. 

After these unit tests, I run the **given test** function. There are actually 6 tests in that function. The former 5 tests have valid inputs and expect the program work normally, don’t even reach boundary cases, so, my program passed these tests. The last one tries to get block from a file that don’t have any data page, therefore, there should be some error messages. My program detected this illegal operation, and print out error message accordingly.

Finally, I wrote a function to test if there is memory leak. This test function will create 3 files first, and allocate 10000 pages for each of them. Then, for each of the pages, add them into buffer pool, and then unpin and dispose them. Finally, close these 3 files and release the memory of buffer pool and metadata direct address table.

Here is the result of valgrind:
![enter image description here](https://dl.dropboxusercontent.com/s/8w59rm3j0x3nlyl/comp115proj_valgrind.png?dl=0)

No memory leak detected.

-------------

> Updated at May 6, 2017

## Part B: The Heap File Layer
### Files in This Part
* Makefile
    This is the Makefile of the whole project, including **part A** and **part B**. We can just type `make` to using this Makefile to compile.

* my_testB.c
    This is my *test file*, I wrote unit tests in this file. I also put the given test function in this file as a function called `given_test_function()`.

* projectB.h / projectB.c
    My implementations of all **HFL functions** are in these files, they are well commented. To implement given API, I also wrote some *helper functions*.

* record.h
    The definition of **record structure** is in this file. 

### How to Compile and Run
Before you compile, you will need to open **my_testB.c** to select which *test function* you want to use. You can comment out other functions, just keep the one you want. By default, I left the `given_test_function()`.

To compile this program, make sure you have all files in the same directory, and cd in this directory, type in `make`. The system will automatically compile the program. The executable file is called **testB**, you can then type in `./testB` to run this program. For **instructions** of Makefile like `make clean` and `make reset`,  please see your screen when execute *make*.

If you want to run **test for part A**, you can still follow the instruction in **Part A**, type in `make testA` to compile, and `./testA` to run test for part A.

### Details
#### 1. Decisions
  * Page Organization
      ![Data page organization diagram](https://dl.dropboxusercontent.com/s/xcx9apto2sd70xg/comp115proj_page.jpg?dl=0)
      
  * Tuple Organization
      In my implementation, I chose to use the given struct in **record.h** file as my tuple format. When I need to insert a new record, I create a *record struct* first, then store the *struct* directly into block. The size of one record struct is `sizeof(record)` which is `16 bytes` in this case. 

#### 3. Changed in Part A

I changed a little bit for part A when I implemented this part.

  1.  Added a new attribute `currentRecID` in `struct metadata`, such that we can keep updating the current recordID of a file that we are scanning by calling `HFL_get_next_rec()` and `HFL_get_first_rec()`.

  2.  Added a parameter of `int *blockID` in `BM_alloc_block()`,  after allocate new block, store the new blockID like this :

  `*blockID = newBlockID;`

  Such that we can immediately know the ID of the new allocated block. 

#### 4. Other Implementing Details

  * Calculation of RecordID
      In my implementation, I just need to set variable `ENTRYLENGTH` (which is 16 in this case), and the program will calculate `pageCapacity` (which is 240 in this case) automatically. Then, when we want to insert a new record in block a with `blockID` at `offset` position, we can calculate `recordID` using the following equation:
      
                `recordID = blockID * pageCapacity + offset ;`
  * Record insertion strategy
      Loop to scan from first page to next, see if there is free space in a page. If there is no page in the file or all pages are full, call `BM_alloc_block()` to create a new page, and insert record in this new page.
      
  * How did I deal with multiple scanDesc
      To handle multiple scanDesc, I defined a struct scanner and a dynamic array of scanner:
  ```
    typedef int scanDesc;
  
    typedef struct _scanner {
        fileDesc fd;
        int currentRid;
    } scanner;
    
    extern scanner *scannerTable;
  ```
  Each scanDesc is map to a scanner in the array, such that we can store `fd` and `current record ID` of each scanDesc.

#### 5. Test Cases
To make sure each of my functions works properly, I wrote unit tests for them. In my test functions, I intentionally tested **boundary cases**, like inserting a record when all pages are full, scan records with some of records deleted or try to get a record with a nonexistent recordID. I also tested if my program can have **multiple scans** running together with no influence to each other (in my `test_scanner()` function).  In addition, I wrote a function called `test_HFL_get_next_rec()` to test if my program can work with a **large number of records** (I tested for 50000 records, insert them, close the file, reopen the file, and scan them).

For the **given test function**, I put it in *my_testB.c* file as a `given_test_function()`.  I segmented this function into several parts, added additional print message and pause, such that it will be more clearly to see results. I also executed a **memory test** for this function, added some additional statements to release memory allocated in heap. 
Here is the result of running test with 
`~ make`
`~ valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./testB`

output:
![comp115projectBmemory](https://dl.dropboxusercontent.com/s/zltkjosx4q65023/comp115Bresult.png?dl=0)
    


----------