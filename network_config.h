#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <string>
#include <vector>
#include <stdexcept>

struct Subnet {
    std::string name;
    std::string network_addr;
    std::string subnet_mask;
    int prefix_length;

    Subnet(const std::string& n, const std::string& addr, const std::string& mask, int prefix)
        : name(n), network_addr(addr), subnet_mask(mask), prefix_length(prefix) {}
};

struct RoutingEntry {
    std::string destination;
    std::string next_hop;
    std::string subnet_mask;
    std::string interface;

    RoutingEntry(const std::string& dest, const std::string& hop, const std::string& mask, const std::string& intf)
        : destination(dest), next_hop(hop), subnet_mask(mask), interface(intf) {}
};

// Predefined subnets for different device types
const std::vector<Subnet> SUBNETS = {
    Subnet("Lighting",   "192.168.1.0",  "255.255.255.192", 26),
    Subnet("Thermostat", "192.168.1.64", "255.255.255.224", 27),
    Subnet("Security",   "192.168.1.96", "255.255.255.240", 28)
};

#endif