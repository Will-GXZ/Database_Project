#include <stdio.h>
#include <string.h>
#include "my_queue.h"

int *queue = NULL;
int Qcapacity = 10;
int Qfront = 0;
int Qcount = 0;

void Q_init() {
    queue = (int *)malloc(Qcapacity * (sizeof(int)));
    Qcount = 0;
    Qfront = 0;
    for (int i = 0; i < Qcapacity; ++i)
    {
        queue[i] = -1; // reset each blockID to -1
    }
}

int Q_isempty() {
    if (Qcount == 0) {
        return 1;
    } else {
        return 0;
    }
}

void Q_doubleCapacity() {
    int *temp = (int *)malloc(2 * Qcapacity * (sizeof(int)));
    memcpy(temp, queue, Qcapacity * sizeof(int));
    free(queue);
    queue = temp;
    temp = NULL;
    Qcapacity *= 2; 
}

void Q_enqueue(int element) {
    if (Qfront + Qcount - 1 > Qcapacity * 0.75) {
        Q_doubleCapacity();
    }
    queue[Qfront + Qcount] = element;
    Qcount ++;
}

int Q_dequeue() {
    int ret = queue[Qfront];
    queue[Qfront] = -1;
    Qcount --;
    Qfront ++;
    if (Qfront > 0.25 * Qcapacity) {
        int *temp = (int *)malloc(Qcapacity * sizeof(int));
        memcpy(temp, queue + Qfront, Qcount * sizeof(int));
        free(queue);
        queue = temp;
        temp = NULL;
        Qfront = 0;
    }
    return ret;
}

void Q_clear() {
    free(queue);
    Q_init();
}

void Q_free() {
    free(queue);
    queue = NULL;
}


