#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netinet/in.h>
#include <pthread.h>

#include "queue.h"

const int BUFFER_SIZE = 1024;
const int FILE_SIZE_BUFFER_SIZE = 4;

Queue *cliQueue;

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t qempty = PTHREAD_COND_INITIALIZER;


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

int receivefile(int clsockfd, char* cppfname) {
    char buff[BUFFER_SIZE];
    int file_size = 0;
    char file_size_bytes[4];

    // receive the file size
    if (recv(clsockfd, &file_size_bytes, sizeof(file_size), 0) == -1) {
        printf("Error in receving file size");
        return -1;
    }
    memcpy(&file_size, file_size_bytes, sizeof(file_size));

    FILE *file = fopen(cppfname, "wb");
    if (!file) {
        printf("Error in creating cpp file\n");
        return -1;
    }

    int total_bytes_recv = 0;
    while(1) {
        size_t fbr = recv(clsockfd, buff, BUFFER_SIZE, 0);
        total_bytes_recv += fbr;
        if (fbr == 0) {
            // file receiving is complete

            break;
        } else if (fbr < 0) {
            printf("Error in receiving file\n");
            fclose(file);
            break;
        }
        fwrite(buff, 1, fbr, file);
        bzero(buff, BUFFER_SIZE);
        if (total_bytes_recv >= file_size)
            break;
    }
    fclose(file);

    return 0;
}

void* compile_and_run() {
    char cppfname[30];
    char errfname[30];
    char opfname[30];
    char exefname[30];
    char compile_cmd[100];
    char run_cmd[100];
    char diff_cmd[100];

    while(1){        
        pthread_mutex_lock(&qmutex);
        
        if(is_empty(cliQueue))
            pthread_cond_wait(&qempty, &qmutex);
        
        int clsockfd = dequeue(cliQueue);
            
        pthread_mutex_unlock(&qmutex);

        sprintf(cppfname, "./grader/src%d.cpp", clsockfd);
        sprintf(errfname, "./grader/err%d.txt", clsockfd);
        sprintf(opfname, "./grader/op%d.txt", clsockfd);
        sprintf(exefname, "./grader/exe%d", clsockfd);

        sprintf(compile_cmd, "g++ -o %s %s 2> %s", exefname, cppfname, errfname);
        sprintf(run_cmd, "./%s 1> %s 2> %s", exefname, opfname, errfname);
        sprintf(diff_cmd, "diff %s exp.txt", opfname);

        if (receivefile(clsockfd, cppfname) == -1) {
            close(clsockfd);
            continue;
        }

        // compile the code
        int status = system(compile_cmd);

        if (status != 0) {
            send_msg_from_file_to_client(clsockfd, errfname);
        } else {
            // check runtime error
            int r_status = system(run_cmd);
            if (r_status != 0) {
                send_msg_from_file_to_client(clsockfd, errfname);
            } else {
                // if no runtime error, the output is saved in op.txt
                int st = system(diff_cmd);
                if (st==0) {
                    send_msg_to_client(clsockfd, "Ran successfully!\n");
                }
            }
        }
        close(clsockfd);
    }          
}

int main(int argc, char* argv[]) {
    int sockfd, portno;
    int thread_pool_size;
    int active_threads = 0;

    cliQueue = createQueue();

    if (argc != 3) {
        printf("Usage: <port-no> <thread pool size>\n");
        exit(0);
    }   

    portno = atoi(argv[1]);
    thread_pool_size = atoi(argv[2]);

    socklen_t cl_arr_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Error creating socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Couldn't bind\n");
        exit(1);
    }

    listen(sockfd, 2);
 
    cl_arr_len = sizeof(cli_addr);
    
    pthread_t thread[thread_pool_size];
    for(int i=0; i<thread_pool_size; i++)
        if (pthread_create(&thread, NULL, compile_and_run, NULL) != 0)
            printf("Failed to create Thread\n");
    

    while(1) {
        int clsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cl_arr_len);

        if (clsockfd < 0) {
            printf("Error accepting\n");
            continue;
        }
        
        pthread_mutex_lock(&qmutex); 
               
        enqueue(cliQueue, clsockfd); 
        pthread_cond_signal(&qempty);
        
        pthread_mutex_unlock(&qmutex);
        
    }
}
