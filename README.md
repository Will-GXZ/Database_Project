# CS115 Database Systems Analysis Class Project

#### Xiaozheng Guo

Language: C

Created at Mar 31, 2017
____________________________

### *Acknowledgements*
I would like to express my very great appreciation to Prof.Athanassoulis for his valuable and constructive suggestions during the planning and development of this work. I would also like to extend my thanks to TAs of this course for their continuous and patient response in [piazza](https://piazza.com). 

I also wish to acknowledge the help provided by [tutorialspoint](https://www.tutorialspoint.com/c_standard_library) and [stackoverflow](https://stackoverflow.com/). To work on this project, I need to review and learn a lot of stuff in C and linux, therefore I search the Internet and these two web sites gave me very useful informations. 

______________________________

### Part A: The buffer manager
#### Files in This Part
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

#### How to Compile 
Before you compile and run this part, you need to edit  **my_test.c** to select which test function you want to run.  Because now I have commented out all code in the **main** function.

To compile this part for testing purpose, make sure you have all the files in the same directory, and type in `make reset` and `make test`. 

#### Details
*  Buffer Management
    1.  Buffer Pool Structure
    This program use a dynamic array as data structure, each slot of the array stores a block structure.
     
     *The definition of the array and block: *
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
   2. Buffer Pool Replacement Policy
   In my project, I choose to use a variant of LRU, called **clock** replacement, which is mentioned in text book. The implementation of this replacement policy is in `buffer_add_block` function in *projectA.c* file. I also write a well commented **test function** in *my_test.c* to test my implementation.
   
*  File Management
   1.  File Metadata
   In my work, I define a **metadata structure** to store informations of each opened file. In the mean time, maintain a **metadata page** for each file in disk. When we open a file, read in its metadata page, store metadata in memory. When we close a file, write back metadata accordingly. 
   This is my definition of metadata structure:
   ```
   typedef struct _metadata {
        int currentID; // the blockID that is current in use
        int firstBlockID;
        int lastBlockID;
        int blockNumber; // the number of total page the file has
        int headerNumber; // the number of header pages of the file.
        char *fileName;
        FILE *fp;
} metadata;
   ```
   
   2. File Organization
   In my implementation, I choose to use the **page directory** strategy to implement **heap file** organization. Specifically, the program  will store each page as a small file of 4096 bytes in disk, and for each database file,  maintain a series of header page as data pages' directory. Also, as I mentioned above, the program will maintain a metadata page for each database file in disk.

        Detailed roganization:
    
    *   Header Page
        ![Header page organization diagram](https://dl.dropboxusercontent.com/s/2xztywx1dmrqjtr/comp115proj_header.jpg?dl=0)
    
    *   Data page
        ![Data page organization diagram](https://dl.dropboxusercontent.com/s/xcx9apto2sd70xg/comp115proj_page.jpg?dl=0)
   
   *    Metadata page
        ![Metadata page orgnization diagram](https://dl.dropboxusercontent.com/s/ahw67mb6mo9jbva/comp115proj_meta.jpg?dl=0)
  
  
  