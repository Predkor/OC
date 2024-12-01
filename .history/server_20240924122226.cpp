#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

using namespace std;

int main() {
    int serverSocket = socket(AF_INET,SOCK_STREAM, 0);
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    listen(serverSocket, 5);

}