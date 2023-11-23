#include "utilityFiles/queue/queue.h"
#include<stdio.h>

int main() {
    Queue *q = createQueue();
    enqueue(q, 1);    
    enqueue(q, 2);
    enqueue(q, 3);
    enqueue(q, 4);
    print_queue(q);
    printf("\n\n%d", dequeue(q));
    printf(" %d", dequeue(q));
    printf(" %d", dequeue(q));
    printf(" %d", dequeue(q));
    printf(" %d", dequeue(q));
    enqueue(q, 1);    
    enqueue(q, 2);
    enqueue(q, 3);
    enqueue(q, 4);
    print_queue(q);
}