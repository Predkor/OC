#include <iostream>
#include <fstream>

#include "httplib.h" // custom lib by yhirose

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

using namespace httplib;
using namespace std;

void svr_home(const Request& req, Response res) {
    fstream file("index.html");
    
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content("HEllo", "text/html");
    } else {
        res.status = 400;
        res.set_content("Page is not found!", "text/plain");
    }
}

int main() {
    Server svr;

    svr.Get("/", svr_home); 

    cout << "Server is listen http://localhost:" << PORT << endl;
    svr.listen("localhost", PORT);

    return 0;
}