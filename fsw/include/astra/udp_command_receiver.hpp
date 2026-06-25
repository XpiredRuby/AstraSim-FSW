#pragma once

#include "astra/command_packet.hpp"

#include <cstdint>
#include <string>

namespace astra {

class UdpCommandReceiver {
public:
    explicit UdpCommandReceiver(
        std::uint16_t listen_port,
        const std::string& bind_ip = "0.0.0.0",
        int timeout_ms = 1000
    );

    ~UdpCommandReceiver();

    UdpCommandReceiver(const UdpCommandReceiver&) = delete;
    UdpCommandReceiver& operator=(const UdpCommandReceiver&) = delete;

    UdpCommandReceiver(UdpCommandReceiver&&) = delete;
    UdpCommandReceiver& operator=(UdpCommandReceiver&&) = delete;

    bool is_ready() const;
    std::uint16_t bound_port() const;

    bool receive_packet(CommandPacket& packet_out) const;

private:
    int socket_fd_;
    std::uint16_t bound_port_;
    bool ready_;
};

}  // namespace astra
