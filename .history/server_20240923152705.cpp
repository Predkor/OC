#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}