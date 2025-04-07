#include "device.h"
#include <sstream>

Device::Device(const std::string& ip, const std::string& mac, const Subnet& sub)
    : ip_address(ip), mac_address(mac), subnet(sub), is_online(true) {}

// Light implementation
Light::Light(const std::string& ip, const std::string& mac, const Subnet& sub)
    : Device(ip, mac, sub), state(false), brightness(100) {}

std::string Light::getStatus() {
    std::stringstream ss;
    ss << "Light [" << ip_address << "]: " 
       << (state ? "ON" : "OFF") 
       << " (Brightness: " << brightness << "%)";
    return ss.str();
}

bool Light::executeCommand(const std::string& command) {
    if (command == "ON") {
        state = true;
        return true;
    }
    else if (command == "OFF") {
        state = false;
        return true;
    }
    else if (command.find("BRIGHTNESS=") == 0) {
        try {
            brightness = std::stoi(command.substr(11));
            brightness = std::max(0, std::min(100, brightness));
            return true;
        }
        catch (...) {
            return false;
        }
    }
    return false;
}

// Thermostat implementation
Thermostat::Thermostat(const std::string& ip, const std::string& mac, const Subnet& sub)
    : Device(ip, mac, sub), current_temp(22.0), target_temp(22.0), is_heating(false) {}

std::string Thermostat::getStatus() {
    std::stringstream ss;
    ss << "Thermostat [" << ip_address << "]: "
       << "Current: " << current_temp << "°C, "
       << "Target: " << target_temp << "°C, "
       << (is_heating ? "Heating" : "Idle");
    return ss.str();
}

bool Thermostat::executeCommand(const std::string& command) {
    if (command.find("SET=") == 0) {
        try {
            target_temp = std::stof(command.substr(4));
            is_heating = current_temp < target_temp;
            return true;
        }
        catch (...) {
            return false;
        }
    }
    return false;
}

// SecurityCamera implementation
SecurityCamera::SecurityCamera(const std::string& ip, const std::string& mac, const Subnet& sub)
    : Device(ip, mac, sub), recording(false), last_motion("Never") {}

std::string SecurityCamera::getStatus() {
    std::stringstream ss;
    ss << "Camera [" << ip_address << "]: "
       << (recording ? "Recording" : "Standby")
       << ", Last motion: " << last_motion;
    return ss.str();
}

bool SecurityCamera::executeCommand(const std::string& command) {
    if (command == "START_RECORDING") {
        recording = true;
        return true;
    }
    else if (command == "STOP_RECORDING") {
        recording = false;
        return true;
    }
    else if (command.find("MOTION_DETECTED=") == 0) {
        last_motion = command.substr(15);
        return true;
    }
    return false;
}