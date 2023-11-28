#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <map>

#include "serverFiles/utilityFiles/queue/queue.h"
#include "serverFiles/utilityFiles/fileshare/fileshare.h"
#include "serverFiles/utilityFiles/error/errors.h"
#include "serverFiles/utilityFiles/request-id/request_id.h"

Queue *cliQueue;
Queue *fileQueue;

pthread_mutex_t qmutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qmutex2 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t qempty1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t qempty2 = PTHREAD_COND_INITIALIZER;

const int STATUS_SUCCESSFUL = 0;
const int STATUS_COMPILER_ERROR = 1;
const int STATUS_RUNTIME_ERROR = 2;

map<string, int> request_status_map;


void send_msg_from_file_to_client(int clsockfd, char* outfilename) {
    // compile error file descriptor
    int cefd = open(outfilename, O_RDONLY);
       
    // // compiler error content
    char ce_cnt[1000];

    // // compile error bytes read
    int cebr = read(cefd, &ce_cnt, 1000);

    write(clsockfd, ce_cnt, cebr);
    close(cefd);
    return;
}

void send_msg_to_client(int clsockfd, char* msg) {
    write(clsockfd, msg, strlen(msg));
}


void handle_status_check_request(int clsockfd) {
    // Ask the client for request id

    char request_id[30];
    char errfname[30];
    recv(clsockfd, request_id, 30, 0);

    int status = request_status_map[string(request_id)];
    if (request_status_map.find(string(request_id)) == request_status_map.end()) {
        send_msg_to_client(clsockfd, "Invalid request id");
        return;
    }
    cout << "reaching here\n";

    if (status == STATUS_COMPILER_ERROR){
        sprintf(errfname, "./grader/err%s.txt", request_id);
        send_msg_from_file_to_client(clsockfd, errfname);
    }  
    else if(status == STATUS_RUNTIME_ERROR) {
        sprintf(errfname, "./grader/err%s.txt", request_id);
        send_msg_from_file_to_client(clsockfd, errfname);
    } else {
        send_msg_to_client(clsockfd, "Ran successfully\n");
    }
}

void* compile_and_run(void* args) {
    char cppfname[100];
    char errfname[100];
    char opfname[100];
    char exefname[100];
    char compile_cmd[200];
    char run_cmd[200];
    char diff_cmd[200];

    while(1){        
        pthread_mutex_lock(&qmutex2);
        
        if(is_queue_empty(cliQueue))
            pthread_cond_wait(&qempty2, &qmutex2);
        
        ClientRequest req = dequeue(cliQueue);
        char* request_id = req.request_id;
            
        pthread_mutex_unlock(&qmutex2);

        sprintf(cppfname, "./grader/src%s.cpp", request_id);
        sprintf(errfname, "./grader/err%s.txt", request_id);
        sprintf(opfname, "./grader/op%s.txt", request_id);
        sprintf(exefname, "./grader/exe%s", request_id);

        sprintf(compile_cmd, "g++ -o %s %s 2> %s", exefname, cppfname, errfname);
        sprintf(run_cmd, "./%s 1> %s 2> %s", exefname, opfname, errfname);
        sprintf(diff_cmd, "diff %s serverFiles/exp.txt", opfname);

        // compile the code
        int status = system(compile_cmd);

        if (status != 0) {
            // Compiler error
            request_status_map.emplace(string(request_id), STATUS_COMPILER_ERROR);
        } else {
            // check runtime error
            int r_status = system(run_cmd);
            if (r_status != 0) {
                // send_msg_from_file_to_client(clsockfd, errfname);
                request_status_map.emplace(string(request_id), STATUS_RUNTIME_ERROR);
            } else {
                // if no runtime error, the output is saved in op.txt
                int st = system(diff_cmd);
                if (st==0) {
                    request_status_map.emplace(string(request_id), STATUS_SUCCESSFUL);
                }
            }
        }
    }          
}

void* file_add_in_queue(void* args) {
    char cppfname[100];
    bzero(cppfname, 100);

    while(1){        
        pthread_mutex_lock(&qmutex1);
        
        if(is_queue_empty(fileQueue))
            pthread_cond_wait(&qempty1, &qmutex1);
        
        ClientRequest req = dequeue(fileQueue);
        int clsockfd = req.sockfd;
        char* request_id = req.request_id;
            
        pthread_mutex_unlock(&qmutex1);

        sprintf(cppfname, "./grader/src%s.cpp", request_id);


        if (receivefile(clsockfd, cppfname) == -1) {
            close(clsockfd);
            continue;
        }

        pthread_mutex_lock(&qmutex2); 
        
        enqueue(cliQueue, clsockfd, request_id);
        pthread_cond_signal(&qempty2);
            
        pthread_mutex_unlock(&qmutex2);
        
        close(clsockfd);
    }          
}

int main(int argc, char* argv[]) {
    char* location = "gradingclient.c - main";
    int sockfd, portno;
    int thread_pool_size;
    int active_threads = 0;

    cliQueue = createQueue();
    fileQueue = createQueue();

    if (argc != 3) {
        error_exit(location, "Usage: <port-no> <thread pool size>", 1);
    }   

    portno = atoi(argv[1]);
    thread_pool_size = atoi(argv[2]);

    socklen_t cl_arr_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error_exit(location, "Error creating socket", 1);
    }

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error_exit(location, "Coudn't bind, Address already in use!", 1);
    }

    listen(sockfd, 2);
 
    cl_arr_len = sizeof(cli_addr);

    thread_pool_size+=1;
    pthread_t thread[thread_pool_size];

    for(int i=0; i<thread_pool_size; i++)
        if(i < (thread_pool_size-1)/2){
            if (pthread_create(&thread[i], NULL, compile_and_run, NULL) != 0)
                printf("Failed to create Thread\n");
        }
        else if(i < thread_pool_size-3){
            if (pthread_create(&thread[i], NULL, file_add_in_queue, NULL) != 0)
                printf("Failed to create Thread\n");
        }
        else if(i == thread_pool_size-2){
            if (pthread_create(&thread[i], NULL, file_add_in_queue, NULL) != 0)
                printf("Failed to create Thread\n");
        }
        else{
            if (pthread_create(&thread[i], NULL, file_add_in_queue, NULL) != 0)
                printf("Failed to create Thread\n");
        }
    
    // return 0;
    
    while(1) {
        int clsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cl_arr_len);

        if (clsockfd < 0) {
            printf("Error accepting\n");
            continue;
        }

        int check = receive_reqType(clsockfd);
        cout << "check = " << check << endl;
        if (check == 1) {
            // new request
            pthread_mutex_lock(&qmutex1); 
        
            // Generate request ID
            char* request_id = generateFormattedTimestampID();
            enqueue(fileQueue, clsockfd, request_id);
            send_msg_to_client(clsockfd, request_id);
            pthread_cond_signal(&qempty1);
            
            pthread_mutex_unlock(&qmutex1);
        } else {
            // status check request
            handle_status_check_request(clsockfd);
        }        
    }
}
