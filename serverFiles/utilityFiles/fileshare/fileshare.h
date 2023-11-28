#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

int receivefile(int clsockfd, char* cppfname);
int send_file(int sockfd, char* fname);
int send_reqID(int sockfd, char* reqID);
int send_reqType(int sockfd, char* type);
int receive_reqType(int clsockfd);
int receive_reqDetails(int clsockfd);