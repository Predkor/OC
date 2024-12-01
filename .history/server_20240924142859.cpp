#include <iostream>
#include "httplib.h"
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <jsoncpp/json/json.h>

using namespace httplib;
using namespace std;

void send_process_list(const Request& req, Response& res) {
    DIR* dir;
    struct dirent* entry;

    dir = opendir("/proc");
    if (dir == NULL) {
        res.status = 500;
        res.set_content("Ошибка при открытии /proc", "text/plain");
        return;
    }

    Json::Value processes(Json::arrayValue);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            Json::Value process;
            process["pid"] = atoi(entry->d_name);
            // Можно добавить больше информации о процессе
            processes.append(process);
        }
    }
    closedir(dir);

    Json::StreamWriterBuilder writer;
    std::string output = Json::writeString(writer, processes);

    res.set_content(output, "application/json");
}

void kill_process(const Request& req, Response& res) {
    if (req.has_param("pid")) {
        int pid = stoi(req.get_param_value("pid"));
        if (kill(pid, SIGKILL) == 0) {
            res.set_content("Процесс завершен успешно.", "text/plain");
        } else {
            res.status = 500;
            res.set_content("Не удалось завершить процесс.", "text/plain");
        }
    } else {
        res.status = 400;
        res.set_content("Не указан параметр pid.", "text/plain");
    }
}

void serve_html(const Request& req, Response& res) {
    ifstream file("index.html");
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    } else {
        res.status = 404;
        res.set_content("HTML файл не найден.", "text/plain");
    }
}

int main() {
    Server svr;

    // Маршруты
    svr.Get("/", serve_html);
    svr.Get("/get_process_list", send_process_list);
    svr.Post("/kill_process", kill_process);

    std::cout << "Сервер запущен на порту 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}