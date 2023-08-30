#include "InetAddress.h"


InetAddress::InetAddress(std::string ip ,uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const 
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof buf);
    return buf;
}

// ip:port
std::string InetAddress::toIpPort() const 
{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof buf);
    size_t len = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + len, ":%u", port);
    return buf;
}

uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}