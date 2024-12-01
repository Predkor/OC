#include <iostream>
#include <crow.h>

#include <fstream>
#include <sstream>
#include <dirent.h> // opendir() and other methode
#include <nlohmann/json.hpp>

#define PORT 8080
#define LOCALHOST "127.0.0.1"

using namespace std;

string getProcessList() {
    DIR* dir;
    struct dirent* entry;
    
    dir = opendir("/proc");
    if (dir == NULL) {
        return "<h"
    }

}

string readHTMLFIle(const string& filePath) {
    ifstream file(filePath);

    if (!file.is_open()) {
        return "<h1>Not found page!</h1>";
    }

    stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

int main() {
    crow::SimpleApp app;

    
    CROW_ROUTE(app, "/")([]() {
        string htmlContent = readHTMLFIle("index.html");
        return crow::response(htmlContent);        
    });

    app.bindaddr(LOCALHOST).port(PORT).multithreaded().run();

    return 0;
}