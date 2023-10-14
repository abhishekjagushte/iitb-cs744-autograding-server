#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include <sys/time.h>


int main(int argc, char *argv[]) {
    char *fname;
    int sockfd = 0;

    if (argc != 7) {
        printf("Usage: <server-IP> <server-port> <file-name> <loop num> <sleep time> <id>\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    struct hostent *server;


    int portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        printf("No such host available\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;

    char fbuff[10000];
    fname = argv[3];
    int fd = open(fname, O_RDONLY);
    int fbr = read(fd, &fbuff, 10000);

    int count = atoi(argv[4]);
    int icount = count;
    int sleep_time = atoi(argv[5]);
    int prog_id = atoi(argv[6]);
    int time_sum = 0;
    int succ = 0;

    // total time taken for loop
    struct timeval total_time_start;
    gettimeofday(&total_time_start, NULL);


    while (count--) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0) {
            printf("Error in creating a socket\n");
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            printf("Couldn't connect!");
            exit(1);
        }

        
        // start time
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        
        int fbw = write(sockfd, fbuff, fbr);

        if (fbw < 0) {
            printf("Error in writing");
        } else {
            succ++;
            char res[1000];
            printf("Waiting for response id = %d\n", prog_id);
            int resbytes = read(sockfd, &res, 1000);
            printf("Got response id = %d\n", prog_id);
            
            // end time
            struct timeval end_time;
            gettimeofday(&end_time, NULL);

            write(STDOUT_FILENO, res, resbytes);

            int t_diff = (end_time.tv_sec*1000 + end_time.tv_usec/1000) - (start_time.tv_sec*1000 + start_time.tv_usec/1000);
            time_sum += t_diff;
        }
        close(sockfd);
        sleep(sleep_time);
    }

    struct timeval total_time_end;
    gettimeofday(&total_time_end, NULL);
    int t_diff = (total_time_end.tv_sec*1000 + total_time_end.tv_usec/1000) - (total_time_start.tv_sec*1000 + total_time_start.tv_usec/1000);

    float average = (float) time_sum/icount;
    printf("Successful %d/%d. Average time taken in prog %d = %f with %d loop iterations. Total time taken for loop = %d ms. Throughput = %f\n", succ, icount, prog_id, average, icount, t_diff, (float) (succ*1000)/t_diff);

    close(fd);
}