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

    if (pw) {
        return string(pw->pw_name);
    }

    return "unknown";
}

json getProcessInfo(const string& pid) {
    json processInfo;
    processInfo["pid"] = pid;

    ifstream file("/proc/" + pid + "/status");
    string line;
    string VmSize;
    uid_t uid = 0;

    while (getline(file, line)) {
        if (line.find("Uid:") == 0) {
            istringstream iss(line.substr(5));
            iss >> uid;        
        }

        if (line.find("VmSize:") == 0) {
            istringstream iss(line.substr(7));
            iss >> VmSize;
            break;
        }
    }

    processInfo["user"] = getUsername(uid);
    processInfo["VmSize"] = atoi(VmSize.c_str()) / 1024;

    ifstream stat_file("/proc/" + pid + "/stat");
    if (stat_file) {
        string comm;
        char state;
        int pr, ni;
        long unsigned utime, stime, vsize, rss;

        stat_file >> processInfo["pid"] >> comm >> state;
        stat_file.ignore(256, ')');

        processInfo["s"] = string({state});
        processInfo["pr"] = pr;
        processInfo["ni"] = ni;
    }   

    ifstream cmdLineFile("/proc/" + pid + "/cmdline");
    if (cmdLineFile) {
        string cmdline;
        getline(cmdLineFile, cmdline, '\0');
        processInfo["command"] = cmdline;
    } else {
        processInfo["command"] = "";
    }

    return processInfo;
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
                processes.push_back(getProcessInfo(name));
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