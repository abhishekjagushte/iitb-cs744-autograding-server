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

#include "serverFiles/utilityFiles/fileshare/fileshare.h"
#include "serverFiles/utilityFiles/error/errors.h"

using namespace std;

struct submit_args {
    int sockfd;
    char* fname;
    int status;
    int prog_id;
};

int create_socket_connection(struct sockaddr_in serv_addr) {
    char* location = "gradingclient.c - create_socket_connection";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_exit(location, "Error in creating a socket", 1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error_exit(location, "Couldn't connect!", 1);
    }

    return sockfd;
}

string getCurrentTimestamp() {
    auto currentTimePoint = std::chrono::system_clock::now();

    std::time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);

    struct tm* localTime = std::localtime(&currentTime);

    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTimePoint.time_since_epoch() % std::chrono::seconds(1)
    );

    char buffer[80]; // Adjust the size if needed
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

    std::string timestamp = std::string(buffer) + "." + std::to_string(micros.count());   

    return timestamp;
}

void write_request_ids_to_file(char* request_id, int prog_id) {
    char write_cmd[100];

    sprintf(write_cmd, "echo \"%s,%s\" >> clientFiles/%d.txt", request_id, getCurrentTimestamp().c_str(), prog_id);
    
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

int send_grading_requests(
    struct sockaddr_in serv_addr, char* fname, int prog_id
) {
    // total time taken for loop
    int sockfd = create_socket_connection(serv_addr);

    int submit_status = 0;
    struct submit_args args = {sockfd, fname, submit_status, prog_id};

    send(sockfd, "new", 4, 0);

    send_file(sockfd, fname);

    char buff[30];
    bzero(buff, 30);

    recv(sockfd, buff, 30, 0);

    write_request_ids_to_file(buff, prog_id);

    close(sockfd);
}

int send_status_requests(
    struct sockaddr_in serv_addr, char* reqID, int prog_id
) {
    int sockfd;
    char* location = "gradingclient.c - send_status_requests";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_exit(location, "Error in creating a socket", 1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error_exit(location, "Couldn't connect!", 1);
    }

    send_reqType(sockfd, "status");

    send_reqID(sockfd, reqID);

    char buff[100];
    bzero(buff, 100);

    recv(sockfd, buff, 100, 0);

    char write_cmd[100];
    sprintf(write_cmd, "echo %s > clientFiles/status_%d.txt", buff, prog_id);
    system(write_cmd);

    receive_reqDetails(sockfd);

    close(sockfd);
}


int main(int argc, char *argv[]) {
    char* location = "gradingclient.c - main";
    char *fname;
    char *req_type;

    if(argc == 6){
        req_type = argv[1];
        if(!strcmp(req_type, "new")){
            if(argc != 6)
                error_exit(location, "Usage: <new> <server-IP> <server-port> <file-name> <id>", 1);
        }
        else if(!strcmp(req_type, "status")){
            if(argc != 6)
                error_exit(location, "Usage: <status> <server-IP> <server-port> <requestID> <id>", 1);
        }
        else
            error_exit(location, "Usage: <new|status> <server-IP> <server-port> <file-name|requestID> <id>", 1);
    }
    else
        error_exit(location, "Usage: <new|status> <server-IP> <server-port> <file-name|requestID> <id>", 1);

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(argv[2]);
    int portno = atoi(argv[3]);
    fname = argv[4];
    int prog_id = atoi(argv[5]);


    if (server == NULL) {
        error_exit(location, "No such host available", 1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;

    int time_sum = 0;
    int succ = 0;

    if(!strcmp(req_type, "new"))
        send_grading_requests(serv_addr, fname, prog_id);
    else{
        send_status_requests(serv_addr, fname, prog_id);
    }
}