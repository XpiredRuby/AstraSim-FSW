#include "astra/udp_telemetry_sender.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace astra {

UdpTelemetrySender::UdpTelemetrySender(
    const std::string& destination_ip,
    std::uint16_t destination_port
)
    : socket_fd_(-1),
      destination_address_(new sockaddr_in{}),
      ready_(false) {
    socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd_ < 0) {
        std::cerr << "Failed to create UDP telemetry socket." << std::endl;
        return;
    }

    std::memset(destination_address_, 0, sizeof(sockaddr_in));
    destination_address_->sin_family = AF_INET;
    destination_address_->sin_port = htons(destination_port);

    const int parse_result = ::inet_pton(
        AF_INET,
        destination_ip.c_str(),
        &(destination_address_->sin_addr)
    );

    if (parse_result != 1) {
        std::cerr << "Invalid UDP destination IP: " << destination_ip << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    ready_ = true;
}

UdpTelemetrySender::~UdpTelemetrySender() {
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
    }

    delete destination_address_;
    destination_address_ = nullptr;
}

bool UdpTelemetrySender::is_ready() const {
    return ready_;
}

bool UdpTelemetrySender::send_packet(const TelemetryPacket& packet) const {
    if (!ready_ || socket_fd_ < 0 || destination_address_ == nullptr) {
        return false;
    }

    const auto bytes = serialize_telemetry_packet(packet);

    const auto sent = ::sendto(
        socket_fd_,
        bytes.data(),
        bytes.size(),
        0,
        reinterpret_cast<const sockaddr*>(destination_address_),
        sizeof(sockaddr_in)
    );

    return sent == static_cast<ssize_t>(bytes.size());
}

}  // namespace astra
