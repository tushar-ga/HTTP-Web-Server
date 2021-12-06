#include "cgihelpers.h"
char *getTimestamp(){
    char *buf = (char*)malloc(sizeof(char) * 1000);
    time_t now = time(0);

    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    
    return buf;
}