#include <stdlib.h>

/* This is my implementation of queue data structure, I use this queue to trace deleted recordID. If a recID is delete, I put it at the back of the queue. if I need to insert a record, I need to check if this queue is empty first, if this queue is empty, than I can insert new record in page with lastBlockID. */

extern int *queue;

void Q_init(void);

void Q_doubleCapacity(void);

void Q_enqueue(int);

int Q_dequeue(void);

int Q_isempty(void);

void Q_clear(void); // reset queue 

void Q_free(void);