#include "queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    q->front = q->rear = NULL;
    return q;
}

Node* create_node(int val, char* request_id) {
    Node *newnode = (Node *) malloc((sizeof(Node)));
    newnode->sockfd = val;
    newnode->request_id = request_id;
    newnode->next = NULL;
    newnode->prev = NULL;
    return newnode;
}

void enqueue(Queue* q, int val, char* request_id) {
    Node *newnode = create_node(val, request_id);
    if (q->front == NULL) {
        q->front = newnode;
        q->rear = newnode;
    } else {
        q->rear->next = newnode;
        newnode->prev = q->rear;
        q->rear = newnode;
    }
}

ClientRequest dequeue(Queue* q) {
    if (q->front == NULL) {
        return {-1, NULL};
    }

    int val = q->front->sockfd;
    char* request_id = q->front->request_id;
    if (q->front == q->rear) {
        q->front = NULL;
        q->rear = NULL;
    } else {
        Node* curr_front = q->front;
        q->front = q->front->next;
        q->front->prev = NULL;

        free(curr_front);
    }

    ClientRequest req;
    req.sockfd = val;
    req.request_id = request_id;

    return req;
}

void print_queue(Queue* q) {
    Node* temp = q->front;
    while (temp != NULL) {
        printf("%d--", temp->sockfd);
        temp=temp->next;
    }
}

_Bool is_queue_empty(Queue* q) {
    return q->front == NULL;
}

long findPos(Queue *q, char *reqID){
    long x=0;
    Node* temp = q->front;

    while(temp != NULL && strcmp(temp->request_id, reqID)){
        temp = temp->next;
        x++;
    }
    if(temp == NULL)    return -1;

    return x;
}