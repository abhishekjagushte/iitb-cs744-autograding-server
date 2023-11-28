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

int create_socket_connection(struct sockaddr_in serv_addr, int timeout) {
    char* location = "gradingclient.c - create_socket_connection"; 
    struct timeval timeout_st;
    timeout_st.tv_sec = timeout;
    timeout_st.tv_usec = 0;

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

    std::string timestamp = std::string(buffer) + "." + std::to_string(micros.count()) + " microseconds";   

    return timestamp;
}

void write_request_ids_to_file(char* request_id, int prog_id) {
    char write_cmd[50];
    sprintf(write_cmd, "echo %s,%s >> clientFiles/%d.txt", request_id, getCurrentTimestamp(), prog_id);
    
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
    struct sockaddr_in serv_addr, int count, char* fname, int sleep_time, int prog_id, int timeout
) {
    int icount = count;
    int succ = 0;
    int time_sum = 0;
    int sockfd;
    int timeouts = 0;
    int errors = 0;

    // total time taken for loop
    struct timeval total_time_start;
    gettimeofday(&total_time_start, NULL);
    
    while (count--) {
        sockfd = create_socket_connection(serv_addr, timeout);

        // start time
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        
        pthread_t timeout_th;
        struct timespec ts;

        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            continue;

        ts.tv_sec += timeout;

        int submit_status = 0;
        struct submit_args args = {sockfd, fname, submit_status, prog_id};

        pthread_create(&timeout_th, NULL, submit, (void *) &args);

        int status = pthread_timedjoin_np(timeout_th, NULL, &ts);
        
        // the updated status for the submitted code is updated in args itself
        submit_status = args.status;


        // end time
        struct timeval end_time;
        gettimeofday(&end_time, NULL);

        int t_diff = (end_time.tv_sec*1000 + end_time.tv_usec/1000) - (start_time.tv_sec*1000 + start_time.tv_usec/1000);
        time_sum += t_diff;

        if (status == ETIMEDOUT) {
            pthread_detach(timeout_th);
            timeouts++;
        } else if (submit_status != 0 || status != 0){
            errors++;
        } else {
            succ++;
        }
        close(sockfd);
        sleep(sleep_time);
    }

    struct timeval total_time_end;
    gettimeofday(&total_time_end, NULL);
    int t_diff = (total_time_end.tv_sec*1000 + total_time_end.tv_usec/1000) - (total_time_start.tv_sec*1000 + total_time_start.tv_usec/1000);

    float average = (float) time_sum/icount;
    printf("Successful %d of %d. Average time taken in prog %d = %f with %d loop iterations. Total time taken for loop = %d ms. Throughput = %f Rate of timeouts = %f Rate of errors = %f\n", succ, icount, prog_id, average, icount, t_diff, (float) (succ*1000)/t_diff, (float) (timeouts*1000)/t_diff, (float) (errors*1000)/t_diff);

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

    receive_reqDetails(sockfd);

    close(sockfd);
}


int main(int argc, char *argv[]) {
    char* location = "gradingclient.c - main";
    char *fname;

    char *req_type;
    if(argc > 5){
        req_type = argv[1];
        if(!strcmp(req_type, "new")){
            if(argc != 9)
                error_exit(location, "Usage: <new> <server-IP> <server-port> <file-name> <loop num> <sleep time> <id> <timeout-in-secs>", 1);
        }
        else if(!strcmp(req_type, "status")){
            if(argc != 6)
                error_exit(location, "Usage: <status> <server-IP> <server-port> <requestID> <id>", 1);
        }
        else
            error_exit(location, "Usage: <new|status> <server-IP> <server-port> <file-name|requestID> <loop num> <sleep time> <id> <timeout-in-secs>", 1);
    } else
        error_exit(location, "Usage: <new|status> <server-IP> <server-port> <file-name|requestID> <loop num> <sleep time> <id> <timeout-in-secs>", 1);

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(argv[2]);
    int portno = atoi(argv[3]);
    fname = argv[4];

    int count;
    int sleep_time;
    int prog_id;
    int timeout;

    if(!strcmp(req_type, "new")){
        count = atoi(argv[5]);
        sleep_time = atoi(argv[6]);
        prog_id = atoi(argv[7]);
        timeout = atoi(argv[8]);
    }
    else{
        prog_id = atoi(argv[5]);
    }


    if (server == NULL) {
        error_exit(location, "No such host available", 1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;

    int time_sum = 0;
    int succ = 0;

    if(!strcmp(req_type, "new"))
        send_grading_requests(serv_addr, count, fname, sleep_time, prog_id, timeout);
    else{
        send_status_requests(serv_addr, fname, prog_id);
    }
}