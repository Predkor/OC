#include <iostream>
#include <crow.h>

#include <fstream>
#include <sstream>

#define PORT 8080
#define LOCALHOST "127.0.0.1"

using namespace std;

string readHTMLFIle(const string& filePath) {
    ifstream file(filePath);

    if (!file.is_open()) {
        return "<h1>Not found page!</h1>";
    }

    stringstream buffer;

    buffer << file.rdbuf();

    

}

int main() {
    crow::SimpleApp app;

    

    app.bindaddr(LOCALHOST).port(PORT).multithreaded().run();

    return 0;
}