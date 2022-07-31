#include "Server.h"
#include <thread>

void work(Server& server) {
    server.processing_requests();
}

int main() {
    // To test images comment out the line below
    TestImages();

    // Create a server and start its work on the other thread
    Server server;
    std::thread th(work, std::ref(server));

    // This thread will wait any symbol
    std::cout << "Write any symbol to turn off the server" << std::endl;
    char c;
    std::cin >> c;
    server.turn_off();
    th.join();
    return 0;
}
