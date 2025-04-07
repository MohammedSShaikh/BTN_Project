#include "router.h"
#include <sstream>
#include <iostream>

Router::Router() {
    // Add default routes for each subnet
    for (const auto& subnet : SUBNETS) {
        addRoute(RoutingEntry(
            subnet.network_addr,
            "0.0.0.0",  // Direct connection
            subnet.subnet_mask,
            "eth0"      // Default interface
        ));
    }
}

unsigned long Router::ipToUlong(const std::string& ip) {
    unsigned long result = 0;
    int shift = 24;  // Start with highest byte
    std::stringstream ss(ip);
    std::string octet;
    
    while (std::getline(ss, octet, '.')) {
        result |= (std::stoul(octet) << shift);
        shift -= 8;
    }
    return result;
}

bool Router::isInSubnet(const std::string& ip, const std::string& network, const std::string& mask) {
    unsigned long ip_ul = ipToUlong(ip);
    unsigned long net_ul = ipToUlong(network);
    unsigned long mask_ul = ipToUlong(mask);
    
    return (ip_ul & mask_ul) == (net_ul & mask_ul);
}

bool Router::routePacket(const std::string& source_ip, const std::string& dest_ip) {
    std::string next_hop = findNextHop(dest_ip);
    if (next_hop.empty()) {
        std::cerr << "No route to host: " << dest_ip << std::endl;
        return false;
    }

    std::string mac;
    if (!arp_table.resolveIP(next_hop == "0.0.0.0" ? dest_ip : next_hop, mac)) {
        std::cerr << "ARP resolution failed for: " << dest_ip << std::endl;
        return false;
    }

    return true;
}

void Router::addRoute(const RoutingEntry& entry) {
    routing_table.push_back(entry);
}

std::string Router::findNextHop(const std::string& dest_ip) {
    for (const auto& route : routing_table) {
        if (isInSubnet(dest_ip, route.destination, route.subnet_mask)) {
            return route.next_hop;
        }
    }
    return "";  // No route found
}

void Router::updateARP(const std::string& ip, const std::string& mac) {
    arp_table.addEntry(ip, mac);
}