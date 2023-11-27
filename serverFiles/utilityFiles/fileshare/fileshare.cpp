#include "fileshare.h"
#include <sys/socket.h>
#include <unistd.h>


const int BUFFER_SIZE = 1024;
const int FILE_SIZE_BUFFER_SIZE = 4;

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

int send_file(int sockfd, char* fname) {
    char buff[BUFFER_SIZE];

    FILE* file = fopen(fname, "r+");
    if (!file) {
        printf("Couldn't open file\n");
        return -1;
    }

    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char file_size_bytes[FILE_SIZE_BUFFER_SIZE];
    memcpy(file_size_bytes, &file_size, sizeof(file_size));

    if (send(sockfd, (char*) &file_size_bytes, sizeof(file_size), 0) == -1) {
        printf("Error in sending file size\n");
        return -1;
    }

    while(!feof(file)) {
        size_t fbr = fread(buff, 1, sizeof(buff), file);

        if (send(sockfd, buff, fbr, 0) == -1) {
            printf("Error in sending file to server\n");
            fclose(file);
            return -1;
        }

        bzero(buff, BUFFER_SIZE);
    }
    fclose(file);
    return 0;
}