#include "server.h"
#include "device.h"
#include "router.h"
#include "network_config.h"
#include <iostream>
#include <string>
#include <thread>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <algorithm>
#include <mutex>
#include <map>
#include <memory>
#pragma comment(lib, "ws2_32.lib")

// Global state and synchronization
std::mutex deviceMutex;
Router router;
std::map<std::string, std::unique_ptr<Device>> devices;

// Initialize devices
void initializeDevices() {
    std::lock_guard<std::mutex> lock(deviceMutex);
    
    // Initialize lights
    devices["192.168.1.10"] = std::make_unique<Light>(
        "192.168.1.10", "00:1A:2B:3C:4D:5E", SUBNETS[0]);
    devices["192.168.1.11"] = std::make_unique<Light>(
        "192.168.1.11", "00:1A:2B:3C:4D:5F", SUBNETS[0]);

    // Initialize thermostats
    devices["192.168.1.65"] = std::make_unique<Thermostat>(
        "192.168.1.65", "00:1A:2B:3C:4D:6A", SUBNETS[1]);
    
    // Initialize security cameras
    devices["192.168.1.97"] = std::make_unique<SecurityCamera>(
        "192.168.1.97", "00:1A:2B:3C:4D:7B", SUBNETS[2]);

    // Update ARP table
    for (const auto& device : devices) {
        router.updateARP(device.first, device.second->getMACAddress());
    }
}

// Process device command and return response
std::string processDeviceCommand(const std::string& ip, const std::string& command) {
    std::lock_guard<std::mutex> lock(deviceMutex);
    
    auto it = devices.find(ip);
    if (it == devices.end()) {
        return "ERROR: Device not found";
    }

    Device* device = it->second.get();
    if (!device->isOnline()) {
        return "ERROR: Device is offline";
    }

    if (device->executeCommand(command)) {
        return device->getStatus();
    }
    
    return "ERROR: Invalid command";
}

void handleClient(SOCKET clientSocket) {
    char buffer[1024] = {0};

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "[Thread] Client disconnected.\n";
            break;
        }

        std::string request(buffer, bytesReceived);
        request.erase(std::remove(request.begin(), request.end(), '\n'), request.end());
        std::cout << "[Thread] Received: " << request << "\n";

        std::string response;

        // Validate request format
        if (request.find("GET /") != 0) {
            response = "ERROR: Invalid request format. Commands must start with 'GET /'\n";
            send(clientSocket, response.c_str(), response.size(), 0);
            continue;
        }

        bool validCommand = false;
        // Parse and handle requests
        if (request == "GET /devices/list") {
            validCommand = true;
            std::lock_guard<std::mutex> lock(deviceMutex);
            response = "Connected devices:\n";
            for (const auto& device : devices) {
                response += device.second->getStatus() + "\n";
            }
        }
        else if (request.find("GET /light/") == 0) {
            validCommand = true;
            std::string ip = "192.168.1.10"; // Default to first light
            if (request.find("/light/2/") == 0) {
                ip = "192.168.1.11";
            }

            if (request.find("/on") != std::string::npos) {
                response = processDeviceCommand(ip, "ON");
            }
            else if (request.find("/off") != std::string::npos) {
                response = processDeviceCommand(ip, "OFF");
            }
            else if (request.find("/status") != std::string::npos) {
                std::lock_guard<std::mutex> lock(deviceMutex);
                auto it = devices.find(ip);
                response = it != devices.end() ? it->second->getStatus() : "ERROR: Light not found";
            }
            else {
                response = "ERROR: Invalid light command. Available commands:\n"
                          "  GET /light/1/on\n"
                          "  GET /light/1/off\n"
                          "  GET /light/1/status\n"
                          "  GET /light/2/on\n"
                          "  GET /light/2/off\n"
                          "  GET /light/2/status";
            }
        }
        else if (request.find("GET /thermostat/") == 0) {
            validCommand = true;
            std::string ip = "192.168.1.65";
            
            if (request.find("/thermostat/set/") != std::string::npos) {
                size_t pos = request.find_last_of('/');
                if (pos != std::string::npos) {
                    std::string temp = request.substr(pos + 1);
                    // Validate temperature input
                    try {
                        float tempValue = std::stof(temp);
                        if (tempValue < 10.0 || tempValue > 30.0) {
                            response = "ERROR: Temperature must be between 10°C and 30°C";
                        } else {
                            response = processDeviceCommand(ip, "SET=" + temp);
                        }
                    } catch (...) {
                        response = "ERROR: Invalid temperature value. Must be a number between 10 and 30";
                    }
                }
            }
            else if (request.find("/status") != std::string::npos) {
                std::lock_guard<std::mutex> lock(deviceMutex);
                auto it = devices.find(ip);
                response = it != devices.end() ? it->second->getStatus() : "ERROR: Thermostat not found";
            }
            else {
                response = "ERROR: Invalid thermostat command. Available commands:\n"
                          "  GET /thermostat/status\n"
                          "  GET /thermostat/set/<temperature>";
            }
        }
        else if (request.find("GET /camera/") == 0) {
            validCommand = true;
            std::string ip = "192.168.1.97";
            
            if (request.find("/status") != std::string::npos) {
                std::lock_guard<std::mutex> lock(deviceMutex);
                auto it = devices.find(ip);
                response = it != devices.end() ? it->second->getStatus() : "ERROR: Camera not found";
            }
            else if (request.find("/record/start") != std::string::npos) {
                response = processDeviceCommand(ip, "START_RECORDING");
            }
            else if (request.find("/record/stop") != std::string::npos) {
                response = processDeviceCommand(ip, "STOP_RECORDING");
            }
            else {
                response = "ERROR: Invalid camera command. Available commands:\n"
                          "  GET /camera/status\n"
                          "  GET /camera/record/start\n"
                          "  GET /camera/record/stop";
            }
        }

        if (!validCommand) {
            response = "ERROR: Unknown command. Available commands:\n"
                      "  GET /devices/list\n"
                      "  GET /light/1/[on|off|status]\n"
                      "  GET /light/2/[on|off|status]\n"
                      "  GET /thermostat/status\n"
                      "  GET /thermostat/set/<temperature>\n"
                      "  GET /camera/status\n"
                      "  GET /camera/record/[start|stop]";
        }

        send(clientSocket, response.c_str(), response.size(), 0);
    }

    closesocket(clientSocket);
}

void runServer(int port) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    // Initialize devices and router
    initializeDevices();
    std::cout << "Devices initialized successfully\n";

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    // Setup server address structure
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Listen for connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Server listening on port " << port << "...\n";

    // Accept and handle client connections
    while (true) {
        sockaddr_in client;
        int clientLen = sizeof(client);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&client, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "New client connected.\n";
        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
}



