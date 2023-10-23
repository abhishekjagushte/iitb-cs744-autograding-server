#include "./queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    q->front = q->rear = NULL;
    return q;
}

Node* create_node(int val) {
    Node *newnode = (Node *) malloc((sizeof(Node)));
    newnode->val = val;
    newnode->next = NULL;
    newnode->prev = NULL;
    return newnode;
}

void enqueue(Queue* q, int val) {
    Node *newnode = create_node(val);
    if (q->front == NULL) {
        q->front = newnode;
        q->rear = newnode;
    } else {
        q->rear->next = newnode;
        newnode->prev = q->rear;
        q->rear = newnode;
    }
}

int dequeue(Queue* q) {
    if (q->front == NULL) {
        return -1;
    }

    int val = q->front->val;
    if (q->front == q->rear) {
        q->front = NULL;
        q->rear = NULL;
    } else {
        Node* curr_front = q->front;
        q->front = q->front->next;
        q->front->prev = NULL;

        free(curr_front);
    }

    return val;
}

void print_queue(Queue* q) {
    Node* temp = q->front;
    while (temp != NULL) {
        printf("%d--", temp->val);
        temp=temp->next;
    }
}

_Bool isEmpty(Queue* q) {
    return q->front == NULL;
}


