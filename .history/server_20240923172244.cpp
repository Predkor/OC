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

    string testCase = "test text";

    string response;
    if (strstr(buffer, "GET / ") != NULL && !htmlContent.empty()) {
        response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + htmlContent;
    } else if (strstr(buffer, "POST /upload") != NULL && !htmlContent.empty()) {
        response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n" + testCase;
    } else {
        response = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n"
                   "<html><body><h1>404 Not Found</h1></body></html>";
    }

    send(clientSocket, response.c_str(), response.length(), 0);

    close(clientSocket);
}

int main() {
    
}