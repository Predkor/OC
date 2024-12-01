#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <cstring>

#define PORT 8080

using namespace std;

string loadPage(const string& path) {
    ifstream file(path);
    
    if (!file.is_open()) {
        cerr << "Failed open file" << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

void handleClient(int clientSocket) {
    char buffer[1024] = {0};
    read(clientSocket, buffer, 1024);

    string htmlContent = loadPage("index.html");

    string response;
    if (strstr(buffer, "GET / ") != NULL && !htmlContent.empty()) {
        response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + htmlContent;
    } else if (strstr(buffer, "POST /upload") != NULL ) {

    } else {

        response = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n"
                   "<html><body><h1>404 Not Found</h1></body></html>";
    }

    send(clientSocket, response.c_str(), response.length(), 0);

    close(clientSocket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    cout << "HTTP server started at http://localhost:" << PORT << endl;

    while (true) {
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        handleClient(client_socket);
    }

    close(server_fd);
    
    return 0;
}