#include "errors.h"

void print_error(char* location[], char* msg) {
    for (int i = 0; location[i] != NULL; i++) {
        printf("%s: ", location[i]);
    }
    printf("%s\n", msg);
}

void error_exit(char* location[], char* msg, int statuscode) {
    print_error(location, msg);
    exit(statuscode);
}