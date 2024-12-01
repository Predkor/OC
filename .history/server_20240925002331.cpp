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

unsigned long getTotalSystemTime() {
    ifstream stat_file("/proc/stat");
    string line;
    if (getline(stat_file, line)) {
        istringstream iss(line);
        string cpu;
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }
    return 0;
}

unsigned long getTotalMemory() {
    ifstream meminfo_file("/proc/meminfo");
    string line;
    unsigned long mem_total = 0;
    while (getline(meminfo_file, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label >> mem_total;
            break;
        }
    }
    return mem_total;
}

json getProcessInfo(const string& pid) {
    json processInfo;
    processInfo["pid"] = pid;

    ifstream file("/proc/" + pid + "/status");
    string line;
    uid_t uid = 0;
    while (getline(file, line)) {
        if (line.find("Uid:") == 0) {
            istringstream iss(line.substr(5));
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

        stat_file >> processInfo["pid"] >> comm >> state >> pr >> ni >> skipws;
        stat_file.ignore(256, ')');

        for (int i = 0; i < 11; ++i) stat_file.ignore(256, ' ');
        stat_file >> utime >> stime;
        for (int i = 0; i < 6; ++i) stat_file.ignore(256, ' ');
        stat_file >> vsize >> rss;

        processInfo["pr"] = pr;
        processInfo["ni"] = ni;
        processInfo["virt"] = vsize / 1024;
        processInfo["res"] = rss * sysconf(_SC_PAGESIZE) / 1024;
        processInfo["%cpu"] = "N/A";

        unsigned long totalProcessTime = utime + stime;
        if (totalProcessTime > 0) {
            processInfo["%cpu"] = fixed << setprecision(2) << (totalProcessTime * 100);
        }

        processInfo["%mem"] = "N/A";
    }

    ifstream cmdLineFile("/proc/" + pid + "/cmdline");
    if (cmdLineFile) {
        string cmdline;
        getline(cmdLineFile, cmdline, '\0');
        processInfo["command"] = cmdline;
    } else {
        processInfo["command"] = "N/A";
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
            std::string name(entry->d_name);
            if (std::all_of(name.begin(), name.end(), ::isdigit)) {
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