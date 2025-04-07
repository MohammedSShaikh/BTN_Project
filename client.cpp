#include "client.h"
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

void displayCommands() {
    std::cout << "Commands:\n"
              << "GET /devices/list\n"
              << "GET /light/[1|2]/[on|off|status]\n"
              << "GET /thermostat/[status|set/<10-30>]\n"
              << "GET /camera/[status|record/start|record/stop]\n"
              << "help - Show commands\n"
              << "exit - Close client\n";
}

void runClient(const char* serverIP, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server. Please make sure the server is running.\n";
        closesocket(sock);
        WSACleanup();
        return;  // Exit the function instead of continuing
    }

    std::cout << "Connected to server successfully!\n";
    displayCommands();

    char buffer[1024] = { 0 };
    while (true) {
        std::cout << "Command: ";
        std::string msg;
        std::getline(std::cin >> std::ws, msg);

        if (msg == "exit") break;
        if (msg == "help") {
            displayCommands();
            continue;
        }

        msg += "\n";
        if (send(sock, msg.c_str(), (int)msg.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send command to server. Connection may be lost.\n";
            break;
        }

        int bytesRead = recv(sock, buffer, 1024, 0);
        if (bytesRead <= 0) {
            std::cerr << "Lost connection to server.\n";
            break;
        }
        
        buffer[bytesRead] = '\0';
        std::cout << "Server: " << buffer;
    }

    closesocket(sock);
    WSACleanup();
}




