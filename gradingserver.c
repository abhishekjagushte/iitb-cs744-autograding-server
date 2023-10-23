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
    while (1) {
        char fbuff[10000];
        char cppfname[30];
        char errfname[30];
        char opfname[30];
        char exefname[30];
        char compile_cmd[60];
        char run_cmd[60];
        char diff_cmd[60];

        int fbr = read(clsockfd, fbuff, 10000);

        if (fbr <= 0) {
            printf("Error in reading client request, ending the connection with fd = %d\n", clsockfd);
            break;
        }

        sprintf(cppfname, "./grader/src%d.cpp", clsockfd);
        sprintf(errfname, "./grader/err%d.txt", clsockfd);
        sprintf(opfname, "./grader/op%d.txt", clsockfd);
        sprintf(exefname, "./grader/exe%d", clsockfd);

        sprintf(compile_cmd, "g++ -o %s %s 2> %s", exefname, cppfname, errfname);
        sprintf(run_cmd, "./%s 1> %s 2> %s", exefname, opfname, errfile);
        sprintf(diff_cmd, "diff %s exp.txt", opfname);

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

        if (clsockfd < 0) {
            printf("Error accepting\n");
        }
        pthread_t thread;
        if (pthread_create(&thread, NULL, compile_and_run, (void *) clsockfd) != 0)
            printf("Failed to create Thread\n");
    }
}