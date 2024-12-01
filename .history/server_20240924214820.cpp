#include <iostream>
#include <crow.h>

#define PORT 8080

using namespace std;

int main() {
    crow::SimpleApp app;

    app.bindaddr("localhost").port(PORT).multithreaded().run();

    return 0;
}