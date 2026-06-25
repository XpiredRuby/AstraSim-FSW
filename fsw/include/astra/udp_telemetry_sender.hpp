#pragma once

#include "astra/telemetry_packet.hpp"

#include <cstdint>
#include <string>

struct sockaddr_in;

namespace astra {

class UdpTelemetrySender {
public:
    UdpTelemetrySender(const std::string& destination_ip, std::uint16_t destination_port);
    ~UdpTelemetrySender();

    UdpTelemetrySender(const UdpTelemetrySender&) = delete;
    UdpTelemetrySender& operator=(const UdpTelemetrySender&) = delete;

    UdpTelemetrySender(UdpTelemetrySender&&) = delete;
    UdpTelemetrySender& operator=(UdpTelemetrySender&&) = delete;

    bool is_ready() const;
    bool send_packet(const TelemetryPacket& packet) const;

private:
    int socket_fd_;
    sockaddr_in* destination_address_;
    bool ready_;
};

}  // namespace astra
