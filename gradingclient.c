#define __USE_GNU

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>

const int BUFFER_SIZE = 1024;
const int FILE_SIZE_BUFFER_SIZE = 4;
struct submit_args {
    int sockfd;
    char* fname;
    int status;
};

void error(char* message) {
    printf("%s\n", message);
    exit(1);
}

int create_socket_connection(struct sockaddr_in serv_addr, int timeout) {
    struct timeval timeout_st;
    timeout_st.tv_sec = timeout;
    timeout_st.tv_usec = 0;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("Error in creating a socket");
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Couldn't connect!");
    }

    return sockfd;
}

int send_file(int sockfd, char* fname) {
    char buff[BUFFER_SIZE];

    FILE* file = fopen(fname, "r+");
    if (!file) {
        error("Couldn't open file");
        return -1;
    }

    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char file_size_bytes[FILE_SIZE_BUFFER_SIZE];
    memcpy(file_size_bytes, &file_size, sizeof(file_size));

    if (send(sockfd, (char*) &file_size_bytes, sizeof(file_size), 0) == -1) {
        error("Error in sending file size");
    }

    while(!feof(file)) {
        size_t fbr = fread(buff, 1, sizeof(buff), file);

        if (send(sockfd, buff, fbr, 0) == -1) {
            error("Error in sending file to server");
            fclose(file);
            return -1;
        }

        bzero(buff, BUFFER_SIZE);
    }
    fclose(file);
    return 0;
}

void* submit(void* args) {
    struct submit_args *args_r = (struct submit_args*) args;
    int sockfd = args_r->sockfd;
    char* fname = args_r->fname;

    if (send_file(sockfd, fname) == -1) {
        args_r->status = -1;
        return (void *) -1;
    }

    char res[1000];
    int resbytes = recv(sockfd, &res, 1000, 0);

    if (resbytes < 0) {
        args_r->status = -1;
    }

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
        struct submit_args args = {sockfd, fname, submit_status};

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

    send_grading_requests(serv_addr, count, fname, sleep_time, prog_id, timeout);
}