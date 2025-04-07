#include "arp.h"

bool ARPTable::resolveIP(const std::string& ip, std::string& mac) {
    std::lock_guard<std::mutex> lock(arp_mutex);
    auto it = ip_to_mac.find(ip);
    if (it != ip_to_mac.end()) {
        mac = it->second;
        return true;
    }
    return false;
}

void ARPTable::addEntry(const std::string& ip, const std::string& mac) {
    std::lock_guard<std::mutex> lock(arp_mutex);
    ip_to_mac[ip] = mac;
}

void ARPTable::removeEntry(const std::string& ip) {
    std::lock_guard<std::mutex> lock(arp_mutex);
    ip_to_mac.erase(ip);
}

void ARPTable::clearTable() {
    std::lock_guard<std::mutex> lock(arp_mutex);
    ip_to_mac.clear();
}

bool ARPTable::exists(const std::string& ip) const {
    return ip_to_mac.find(ip) != ip_to_mac.end();
}