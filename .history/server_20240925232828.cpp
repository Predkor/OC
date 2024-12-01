#include <iostream>
#include <crow.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h> // opendir() and other methode
#include <nlohmann/json.hpp> // best lib for json
#include <pwd.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <unistd.h>

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
    processInfo["VmSize"] = atoi(VmSize.c_str());

    ifstream stat_file("/proc/" + pid + "/stat");
    if (stat_file) {
        string comm;
        char state;
        int pr, ni;
        long unsigned utime, stime, vsize, rss;

        stat_file >> processInfo["pid"] >> comm >> state;
        stat_file.ignore(256, ' ');

        for (int i = 0; i < 14; ++i) {
            stat_file.ignore(256, ' ');
        }
        stat_file >> pr >> ni;

        processInfo["command"] = comm;
        processInfo["s"] = string({state});
        processInfo["pr"] = pr;
        processInfo["ni"] = ni;
    }   

    // ifstream cmdLineFile("/proc/" + pid + "/cmdline");
    // if (cmdLineFile) {
    //     string cmdline;
    //     getline(cmdLineFile, cmdline, '\0');
    //     processInfo["command"] = cmdline;
    // } else {
    //     processInfo["command"] = "";
    // }

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

json killProcess(const string& pid) {
    json result;

    pid_t p = atoi(pid.c_str());

    if (p <= 0) {
        result["status"] = "error";
        result["message"] = "Invalid PID";

        return result;
    }

    int status = kill(p, SIGTERM);

    if (status == 0) {
        result["status"] = "success";
        result["message"] = "Process killed";
    } else {
        result["status"] = "error";
        result["message"] = "Failed to kill";
    }

    return result;
}

json getElfFileContent(const string& filepath) {
    json elfContent;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        cerr << "ELF library initialization failed: " << elf_errmsg(-1) << endl;
        return elfContent;
    }

    int fd = open(filepath.c_str(), O_RDONLY, 0);
    if (fd < 0) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return elfContent;
    }

    Elf *elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (!elf) {
        std::cerr << "elf_begin() failed: " << elf_errmsg(-1) << std::endl;
        close(fd);
        return elfContent;
    }

    GElf_Ehdr ehdr;
    if (gelf_getehdr(elf, &ehdr) == nullptr) {
        std::cerr << "gelf_getehdr() failed: " << elf_errmsg(-1) << std::endl;
        elf_end(elf);
        close(fd);
        return elfContent;
    }

    elfContent["entry_point"] = ehdr.e_entry;
    elfContent["machine"] = ehdr.e_machine;
    elfContent["type"] = ehdr.e_type;

    size_t shstrndx;
    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
        std::cerr << "elf_getshdrstrndx() failed: " << elf_errmsg(-1) << std::endl;
        elf_end(elf);
        close(fd);
        return elfContent;
    }

    Elf_Scn *section = nullptr;
    GElf_Shdr shdr;
    json sections = json::array();
    while ((section = elf_nextscn(elf, section)) != nullptr) {
        if (gelf_getshdr(section, &shdr) != &shdr) {
            std::cerr << "gelf_getshdr() failed: " << elf_errmsg(-1) << std::endl;
            continue;
        }

        const char *section_name = elf_strptr(elf, shstrndx, shdr.sh_name);
        json section_info;
        section_info["name"] = section_name ? section_name : "<unnamed>";
        section_info["size"] = shdr.sh_size;
        section_info["addr"] = shdr.sh_addr;
        section_info["offset"] = shdr.sh_offset;
        sections.push_back(section_info);
    }

    elfContent["sections"] = sections;

    elf_end(elf);
    close(fd);

    return elfContent;
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

    CROW_ROUTE(app, "/killproc").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        string body = req.body;
        auto jsonBody = json::parse(body);
        string pid = jsonBody["pid"];

        auto statusKill = killProcess(pid);

        return crow::response(json(statusKill).dump());
    });

    CROW_ROUTE(app, "/upload").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto fileContent = req.body;



    });

    app.bindaddr(LOCALHOST).port(PORT).multithreaded().run();

    return 0;
}