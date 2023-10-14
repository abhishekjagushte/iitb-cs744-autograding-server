#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netinet/in.h>
#include <pthread.h>

char *errfile = "err.txt";

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

void* compile_and_run(void* fd) {
    int clsockfd = (int) fd;
    char fbuff[10000];
    char cppfname[20];
    char errfname[20];
    char opfname[20];
    char compile_cmd[60];
    char run_cmd[60];
    char diff_cmd[60];


    printf("Server: waiting to receive file for fd = %d\n", clsockfd);

    int fbr = read(clsockfd, fbuff, 10000);

    printf("Server: received file for fd = %d\n", clsockfd);

    sprintf(cppfname, "src%d.cpp", clsockfd);
    sprintf(errfname, "err%d.cpp", clsockfd);
    sprintf(opfname, "op%d.cpp", clsockfd);
    
    sprintf(compile_cmd, "g++ -o exe%d src%d.cpp 2> err%d.txt", clsockfd, clsockfd, clsockfd);
    sprintf(run_cmd, "./exe%d 1> op%d.txt 2> err%d.txt", clsockfd, clsockfd, clsockfd);
    sprintf(diff_cmd, "diff op%d.txt exp.txt", clsockfd);

    // for (int i = 0; i < fbr; i++) {
    //     printf("%c", fbuff[i]);
    // }

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
    close(clsockfd);
}

int main(int argc, char* argv[]) {
    int sockfd, portno;

    if (argc != 2) {
        printf("Usage: <port-no>\n");
        exit(0);
    }   

    portno = atoi(argv[1]);

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

    while(1) {
        int clsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cl_arr_len);

        printf("New connection!! fd = %d\n", clsockfd);

        if (clsockfd < 0) {
            printf("Error accepting\n");
        }
        pthread_t thread;
        if (pthread_create(&thread, NULL, compile_and_run, (void *) clsockfd) != 0)
            printf("Failed to create Thread\n");
    }
}