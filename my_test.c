#include <assert.h>
#include <memory.h>
#include "projectA.h"

void test_init_block(void);

int main(void) {
    printf("############ linking successful ###########\n");

    // test_init_block();

    return 0;
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




































