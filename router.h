#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <string>
#include "network_config.h"
#include "arp.h"

class Router {
private:
    std::vector<RoutingEntry> routing_table;
    ARPTable arp_table;
    
    bool isInSubnet(const std::string& ip, const std::string& network, const std::string& mask);
    unsigned long ipToUlong(const std::string& ip);

public:
    Router();
    bool routePacket(const std::string& source_ip, const std::string& dest_ip);
    void addRoute(const RoutingEntry& entry);
    std::string findNextHop(const std::string& dest_ip);
    void updateARP(const std::string& ip, const std::string& mac);
};

#endif