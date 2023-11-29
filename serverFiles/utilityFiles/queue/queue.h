#include <stdbool.h>

struct Node {
    int val;
    struct Node* next;
    struct Node* prev;
};

typedef struct Node Node;

struct Queue {
    Node* front;
    Node* rear;
    int size;
};

typedef struct Queue Queue;

Queue* createQueue();
void enqueue(Queue* q, int val);
int dequeue(Queue* q);
_Bool is_empty(Queue* q);
Node* create_node(int val);
void print_queue(Queue* q);