#include "errors.h"

void print_error(char* location, char* msg) {
    printf("%s: ", location);
    printf("%s\n", msg);
}

void error_exit(char* location, char* msg, int statuscode) {
    print_error(location, msg);
    exit(statuscode);
}