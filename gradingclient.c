#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>


void error(char* message) {
    printf("%s\n", message);
    exit(1);
}

int create_socket_connection(struct sockaddr_in serv_addr, int timeout) {
    struct timeval timeout_st;
    timeout_st.tv_sec = timeout;
    timeout_st.tv_usec = 0;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // set timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_st, sizeof(timeout_st)) < 0) {
        error("Error setting timeout value");
    }

    if (sockfd < 0) {
        error("Error in creating a socket");
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Couldn't connect!");
    }

    return sockfd;
}

void send_files_to_server(
    struct sockaddr_in serv_addr, int count, char* fname, int sleep_time, int prog_id, int timeout
) {
    int icount = count;
    char fbuff[10000];
    int fd = open(fname, O_RDONLY);
    int fbr = read(fd, &fbuff, 10000);
    close(fd);
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

        int fbw = send(sockfd, fbuff, fbr, 0);

        if (fbw < 0) {
            printf("Error in writing\n");
        } else {
            char res[1000];
                
            int resbytes = recv(sockfd, &res, 1000, 0);

            // end time
            struct timeval end_time;
            gettimeofday(&end_time, NULL);

            int t_diff = (end_time.tv_sec*1000 + end_time.tv_usec/1000) - (start_time.tv_sec*1000 + start_time.tv_usec/1000);
            time_sum += t_diff;

            if (resbytes <= 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    timeouts++;
                } else {
                    errors++;
                }
                continue;
            }

            succ++;
            write(STDOUT_FILENO, res, resbytes);
        }
        close(sockfd);
        sleep(sleep_time);
    }

    struct timeval total_time_end;
    gettimeofday(&total_time_end, NULL);
    int t_diff = (total_time_end.tv_sec*1000 + total_time_end.tv_usec/1000) - (total_time_start.tv_sec*1000 + total_time_start.tv_usec/1000);

    float average = (float) time_sum/icount;
    printf("Successful %d of %d. Average time taken in prog %d = %f with %d loop iterations. Total time taken for loop = %d ms. Throughput = %f No. of timeouts = %d No. of errors = %d\n", succ, icount, prog_id, average, icount, t_diff, (float) (succ*1000)/t_diff, errors, errors);

}


int main(int argc, char *argv[]) {
    char *fname;
    int sockfd = 0;

    if (argc != 8) {
        printf("Usage: <server-IP> <server-port> <file-name> <loop num> <sleep time> <id> <timeout-in-secs>\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(argv[1]);
    int portno = atoi(argv[2]);
    fname = argv[3];
    int count = atoi(argv[4]);
    int sleep_time = atoi(argv[5]);
    int prog_id = atoi(argv[6]);
    int timeout = atoi(argv[7]);

    if (server == NULL) {
        error("No such host available");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_port = htons(portno);
    serv_addr.sin_family = AF_INET;

    int time_sum = 0;
    int succ = 0;

    send_files_to_server(serv_addr, count, fname, sleep_time, prog_id, timeout);
}