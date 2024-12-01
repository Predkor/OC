#include <iostream>
#include <crow.h>

#define PORT 8080

using namespace std;

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, World!";
    });

    app.port(PORT).multithreaded().run();

    return 0;
}