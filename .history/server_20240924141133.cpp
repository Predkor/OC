#include <iostream>

#include "httplib.h" // custom lib by yhirose

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

using namespace httplib;

void svr_home() {
    
}

int main() {
    Server svr;

    svr.Get('/', svr_home);    

    return 0;
}