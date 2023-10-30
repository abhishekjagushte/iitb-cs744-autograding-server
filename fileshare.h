#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


int receivefile(int clsockfd, char* cppfname);
int send_file(int sockfd, char* fname);