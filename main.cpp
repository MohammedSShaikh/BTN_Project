#include "server.h"
#include "client.h"
#include <iostream>

int main() {
    int choice;
    std::cout << "1. Run Server\n2. Run Client\n> ";
    std::cin >> choice;
    std::cin.ignore(); // Clear newline from input buffer

    if (choice == 1) {
        runServer(8080);
    }
    else {
        runClient("127.0.0.1", 8080);
    }

    return 0;
}