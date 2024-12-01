#include <iostream>
#include <crow.h>

#define PORT 8080
#define LOCALHOST "127.0.0.1"

using namespace std;

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, World!";
    });

    app.bindaddr(LOCALHOST).port(PORT).multithreaded().run();

    return 0;
}