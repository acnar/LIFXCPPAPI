#include <iostream>
#include <cassert>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#include "Socket.h"
#include "Packet.h"

namespace lifx {
class UnixSocket: public Socket {
public:
    UnixSocket(const std::string& ip, uint16_t port, bool broadcast) {
        int ret;
        if (!broadcast) {
            base_url = "http://" + ip + ":" + std::to_string(port) + "/";
            write_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            memset(&write_addr, 0, sizeof(write_addr));
            write_addr.sin_family = AF_INET;
            inet_aton(ip.c_str(), &write_addr.sin_addr);
            write_addr.sin_port = htons(port);
            ret = connect(write_sock, (sockaddr *) &write_addr, sizeof(write_addr));
            assert(ret == 0);
        } else {
            write_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            assert(write_sock >= 0);
            memset(&write_addr, 0, sizeof(write_addr));
            write_addr.sin_family = AF_INET;
            inet_aton(ip.c_str(), &write_addr.sin_addr);
            write_addr.sin_port = htons(port);
            int broadcastEnable = 1;
            ret = setsockopt(write_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                    sizeof(broadcastEnable));
            assert(ret >= 0);

            read_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            assert(read_sock >= 0);
            memset(&read_addr, 0, sizeof(read_addr));
            inet_aton("0.0.0.0", &read_addr.sin_addr);
            read_addr.sin_family = AF_INET;
            read_addr.sin_port = htons(port);
            broadcastEnable = 1;
            ret = setsockopt(read_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                    sizeof(broadcastEnable));
            assert(ret >= 0);
            
            ret = bind(read_sock, (sockaddr *) &read_addr, sizeof(read_addr));
            assert(ret >= 0);
        }
    }
    
    void Send(const std::string& data) {
        int bytes;
        std::string message = std::string(data);
        
        do {
            bytes = write(write_sock,message.c_str(),message.length());
            message = message.substr(bytes, message.length());
            if (bytes < 0)
                perror("ERROR writing message to socket.\n");
            if (bytes == 0)
                break;
        } while (message.length() > 0);

    }
    
    std::string Receive(int bytes)
    {
        std::string output(bytes, 0);
        if (read(write_sock, &output[0], bytes-1)<0) {
            perror("Failed to read data from socket.\n");
        }
        return output;
    }   

    void Send(const Packet& packet) {
        int written = sendto(write_sock, (const void*) &packet,
                packet.GetSize(), 0, (struct sockaddr *) &write_addr,
                sizeof(write_addr));
        if (written < 0) {
            perror("sendto");
            exit(1);
        }
    }

    bool Receive(Packet& packet) {
        unsigned size = sizeof(read_addr);
        int r = recvfrom(read_sock, (void*) &packet, sizeof(packet), MSG_DONTWAIT,
                (struct sockaddr *) &read_addr, (socklen_t*) &size);
        
        return (r > 0);
    }

    unsigned GetTicks() const {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    }

protected:
    std::string base_url;
    int write_sock, read_sock;
    struct sockaddr_in write_addr;
    struct sockaddr_in read_addr;
};

Socket* Socket::CreateBroadcast(const std::string& broadcastIP, uint16_t port) {
    return new UnixSocket(broadcastIP, port, true);
}

Socket* Socket::CreateStream(const std::string& ip, uint16_t port) {
    return new UnixSocket(ip, port, false);
}
}
