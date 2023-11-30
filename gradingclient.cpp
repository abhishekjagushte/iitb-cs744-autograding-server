#define __USE_GNU

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <chrono>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "serverFiles/utilityFiles/fileshare/fileshare.h"
#include "serverFiles/utilityFiles/error/errors.h"

using namespace std;

struct submit_args {
    int sockfd;
    char* fname;
    int status;
    int prog_id;
};

int create_socket_connection(char* server_ip_arg, char* portno_arg) {
    char* location = "gradingclient.c - create_socket_connection";

    char *server_ip;
    server_ip = server_ip_arg;
    int portno = atoi(portno_arg);

    if (server_ip == NULL) {
        error_exit(location, "No such host available", 1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_exit(location, "Error in creating a socket", 1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error_exit(location, "Couldn't connect!", 1);
    }

    return sockfd;
}

void write_request_ids_to_file(char* request_id, int prog_id) {
    char write_cmd[100];

    sprintf(write_cmd, "echo %s >> clientFiles/%d.txt", request_id, prog_id);
    
    system(write_cmd);
}

void* submit(void* args) {
    struct submit_args *args_r = (struct submit_args*) args;
    int sockfd = args_r->sockfd;
    char* fname = args_r->fname;
    int prog_id = args_r->prog_id;

    if (send_file(sockfd, fname) == -1) {
        args_r->status = -1;
        return (void *) -1;
    }

    char res[1000];
    int resbytes = recv(sockfd, &res, 1000, 0);

    if (resbytes < 0) {
        args_r->status = -1;
    }

    // Accept the request ID
    write_request_ids_to_file(res, prog_id);
    args_r->status = 0;
}

void send_grading_requests(
    int sockfd, char* fname, int prog_id
) {
    int submit_status = 0;
    struct submit_args args = {sockfd, fname, submit_status, prog_id};

    send(sockfd, "new", 4, 0);

    send_file(sockfd, fname);

    char buff[30];
    bzero(buff, 30);

    recv(sockfd, buff, 30, 0);

    printf("%s\n", buff);

    write_request_ids_to_file(buff, prog_id);

    close(sockfd);
}

void send_status_requests(
    int sockfd, char* reqID, int prog_id
) {
    char* location = "gradingclient.c - send_status_requests";

    send_reqType(sockfd, "status");

    send_reqID(sockfd, reqID);

    char* temp = receive_reqDetails(sockfd);

    char write_cmd[100];

    sprintf(write_cmd, "echo \"%s\" > clientFiles/status%d.txt", temp, prog_id);
    
    system(write_cmd);

    close(sockfd);
}


int main(int argc, char *argv[]) {
    char* location = "gradingclient.c - main";
    char *fname;
    char *req_type;
    
    if(argc != 6){
        error_exit(location, "Usage: <new|status> <server-IP> <server-port> <file-name|requestID> <id>", 1);
    }

    req_type = argv[1];
    fname = argv[4];
    int prog_id = atoi(argv[5]);
    int time_sum = 0;
    int succ = 0;

    int sockfd = create_socket_connection(argv[2], argv[3]);

    if(!strcmp(req_type, "new"))
        send_grading_requests(sockfd, fname, prog_id);
    else{
        send_status_requests(sockfd, fname, prog_id);
    }
}