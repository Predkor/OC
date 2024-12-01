#include <iostream>
#include <crow.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h> // opendir() and other methode
#include <nlohmann/json.hpp> // best lib for json
#include <pwd.h>

#define PORT 8080
#define LOCALHOST "127.0.0.1"

using namespace std;
using json = nlohmann::json;

string getUsername(uid_t uid) {
    struct passwd* pw = getpwuid(uid);

    
}

vector<json> getProcessList() {
    vector<json> processes;
    DIR *dir;
    struct dirent* entry;

    dir = opendir("/proc");
    if (dir == NULL) {
        return processes;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            string name(entry->d_name);
            if (all_of(name.begin(), name.end(), ::isdigit)) {
                json processInfo;
                processInfo["pid"] = stoi(name);

                ifstream cmdLineFile("/proc/" + name + "/cmdline");
                if (cmdLineFile) {
                    string cmdline;
                    getline(cmdLineFile, cmdline);
                    processInfo["cmdline"] = cmdline;
                } else {
                    processInfo["cmdline"] = "N/A";
                }

                processes.push_back(processInfo);

            }
        }
    }

    closedir(dir);

    return processes;
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

    CROW_ROUTE(app, "/processes")([]() {
        auto processes = getProcessList();
        return crow::response(json(processes).dump());
    });

    app.bindaddr(LOCALHOST).port(PORT).multithreaded().run();

    return 0;
}