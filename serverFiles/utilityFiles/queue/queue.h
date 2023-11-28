#include <stdbool.h>
#include <string>

using namespace std;

struct Node {
    int sockfd;
    char* request_id;
    struct Node* next;
    struct Node* prev;
};

struct ClientRequest {
    int sockfd;
    char* request_id;
};

typedef struct ClientRequest ClientRequest;

typedef struct Node Node;

struct Queue {
    Node* front;
    Node* rear;
};

typedef struct Queue Queue;


Queue* createQueue();
void enqueue(Queue* q, int val, char* request_id);
ClientRequest dequeue(Queue* q);
_Bool is_queue_empty(Queue* q);
Node* create_node(int val, char* request_id);
void print_queue(Queue* q);