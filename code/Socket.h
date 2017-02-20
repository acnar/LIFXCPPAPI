#pragma once
#include <string>
#include <cstdint>

namespace lifx {
class Packet;

class Socket {
public:
    static const uint16_t DefaultBroadcastPort = 56700;
    static const uint16_t DefaultStreamPort = 8080;
    static Socket* CreateBroadcast(const std::string& broadcastIP,
            uint16_t port = DefaultBroadcastPort);
    static Socket* CreateStream(const std::string& targetIP, uint16_t port =
            DefaultStreamPort);

    virtual ~Socket() {
    }

    virtual void Send(const std::string& data) = 0;
    virtual void Send(const Packet& packet) = 0;
    virtual std::string Receive(int bytes) = 0;
    virtual bool Receive(Packet& packet) = 0;
    virtual unsigned GetTicks() const = 0;
};
}
