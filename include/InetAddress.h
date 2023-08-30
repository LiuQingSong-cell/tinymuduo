#ifndef INETADDRESS__H__
#define INETADDRESS__H__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>


class InetAddress
{
    public:
        InetAddress(std::string ip = "127.0.0.1" ,uint16_t port = 8000);
        explicit InetAddress(sockaddr_in& addr): addr_(addr){}
        std::string toIp() const;
        std::string toIpPort() const;
        uint16_t port() const;
        const sockaddr_in* getSockAddr() const {return &addr_;}

        void setSockAddr(const sockaddr_in& addr) {addr_ = addr;}
        
    private:
        sockaddr_in addr_;
};


#endif