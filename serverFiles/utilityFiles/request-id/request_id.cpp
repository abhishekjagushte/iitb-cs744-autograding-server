#include "request_id.h"

// Function to generate a timestamp with "YYYYMMDDhhmmssuuuuuu" format
char* generateFormattedTimestampID() {
    struct timeval tv;
    char* id = (char*)malloc(23); // Allocate memory for the ID (adjust the size as needed)

    gettimeofday(&tv, NULL);

    // Get the current time
    struct tm* time_info;
    time_t seconds = (time_t)tv.tv_sec;
    time_info = localtime(&seconds);

    // Format the timestamp as required
    snprintf(id, 23, "%04d%02d%02d%02d%02d%02d%06ld",
             time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
             time_info->tm_hour, time_info->tm_min, time_info->tm_sec, tv.tv_usec);

    return id;
}
