#include <iostream>
#include <crow.h>

#define PORT 8080

using namespace std;

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, World!";
    });

    app.bindaddr("").port(PORT).ssl_file("certfile.crt","keyfile.key").multithreaded().run();

    return 0;
}