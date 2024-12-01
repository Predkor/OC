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
    uid_t uid = 0;
    while (getline(file, line)) {
        if (line.find("Uid:") == 0) {
            std::istringstream iss(line.substr(5));
            iss >> uid;
            break;
        }
    }
    processInfo["user"] = getUsername(uid);

    ifstream stat_file("/proc/" + pid + "/stat");
    if (stat_file) {
        string comm;
        char state;
        int pr, ni;
        long unsigned utime, stime, vsize, rss;
        stat_file >> process_info["pid"] >> comm >> state >> pr >> ni >> std::skipws;
        stat_file.ignore(256, ')');
        for (int i = 0; i < 11; ++i) stat_file.ignore(256, ' ');
        stat_file >> utime >> stime;
        for (int i = 0; i < 6; ++i) stat_file.ignore(256, ' ');
        stat_file >> vsize >> rss;

        process_info["pr"] = pr;
        process_info["ni"] = ni;
        process_info["virt"] = vsize / 1024;
        process_info["res"] = rss * sysconf(_SC_PAGESIZE) / 1024;
        process_info["shr"] = "N/A";
        process_info["%cpu"] = "N/A";
        process_info["%mem"] = "N/A";
    }

    std::ifstream cmdline_file("/proc/" + pid + "/cmdline");
    if (cmdline_file) {
        std::string cmdline;
        std::getline(cmdline_file, cmdline, '\0');
        process_info["command"] = cmdline;
    } else {
        process_info["command"] = "N/A";
    }

    return process_info;
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