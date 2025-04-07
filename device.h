#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <memory>
#include "network_config.h"

class Device {
protected:
    std::string ip_address;
    std::string mac_address;
    Subnet subnet;
    bool is_online;

public:
    Device(const std::string& ip, const std::string& mac, const Subnet& sub);
    virtual ~Device() = default;
    
    virtual std::string getStatus() = 0;
    virtual bool executeCommand(const std::string& command) = 0;
    
    std::string getIPAddress() const { return ip_address; }
    std::string getMACAddress() const { return mac_address; }
    bool isOnline() const { return is_online; }
    void setOnline(bool status) { is_online = status; }
};

class Light : public Device {
private:
    bool state;  // true = ON, false = OFF
    int brightness;  // 0-100%

public:
    Light(const std::string& ip, const std::string& mac, const Subnet& sub);
    std::string getStatus() override;
    bool executeCommand(const std::string& command) override;
};

class Thermostat : public Device {
private:
    float current_temp;
    float target_temp;
    bool is_heating;

public:
    Thermostat(const std::string& ip, const std::string& mac, const Subnet& sub);
    std::string getStatus() override;
    bool executeCommand(const std::string& command) override;
};

class SecurityCamera : public Device {
private:
    bool recording;
    std::string last_motion;

public:
    SecurityCamera(const std::string& ip, const std::string& mac, const Subnet& sub);
    std::string getStatus() override;
    bool executeCommand(const std::string& command) override;
};

#endif