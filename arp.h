#ifndef ARP_H
#define ARP_H

#include <string>
#include <map>
#include <mutex>
#include <optional>

class ARPTable {
private:
    std::map<std::string, std::string> ip_to_mac;
    std::mutex arp_mutex;

public:
    bool resolveIP(const std::string& ip, std::string& mac);
    void addEntry(const std::string& ip, const std::string& mac);
    void removeEntry(const std::string& ip);
    void clearTable();
    bool exists(const std::string& ip) const;
};

#endif