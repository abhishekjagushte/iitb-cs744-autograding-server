#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netinet/in.h>
#include <pthread.h>

#include "queue.h"

char *errfile = "err.txt";

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

void* compile_and_run() {
while(1){
    pthread_mutex_lock(&qmutex);
    
    if(is_empty(cliQueue))
    	pthread_cond_wait(&qempty, &qmutex);
      
    int clsockfd = dequeue(cliQueue);
        
    pthread_mutex_unlock(&qmutex);
    
    while (1) {
        char fbuff[10000];
        char cppfname[20];
        char errfname[20];
        char opfname[20];
        char compile_cmd[60];
        char run_cmd[60];
        char diff_cmd[60];

        int queueSize = 0;
        int sumQueueSize = 0;

        int fbr = read(clsockfd, fbuff, 10000);

        if (fbr <= 0) {
            printf("Error in reading client request, ending the connection with fd = %d\n", clsockfd);
            break;
        }

        sprintf(cppfname, "src%d.cpp", clsockfd);
        sprintf(errfname, "err%d.cpp", clsockfd);
        sprintf(opfname, "op%d.cpp", clsockfd);

        sprintf(compile_cmd, "g++ -o exe%d src%d.cpp 2> err%d.txt", clsockfd, clsockfd, clsockfd);
        sprintf(run_cmd, "./exe%d 1> op%d.txt 2> err%d.txt", clsockfd, clsockfd, clsockfd);
        sprintf(diff_cmd, "diff op%d.txt exp.txt", clsockfd);

        int cppfd = creat(cppfname, 00700);
        int fbw = write(cppfd, fbuff, fbr);

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
        close(cppfd);
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
        printf("Usage: <port-no>\n");
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
        }
        
        pthread_mutex_lock(&qmutex); 
               
        enqueue(cliQueue, clsockfd); 
        pthread_cond_signal(&qempty);
        
        pthread_mutex_unlock(&qmutex);
        
    }
}
