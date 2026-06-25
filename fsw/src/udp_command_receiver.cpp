#include "astra/udp_command_receiver.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>
#include <vector>

namespace astra {

UdpCommandReceiver::UdpCommandReceiver(
    std::uint16_t listen_port,
    const std::string& bind_ip,
    int timeout_ms
)
    : socket_fd_(-1),
      bound_port_(0),
      ready_(false) {
    socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd_ < 0) {
        std::cerr << "Failed to create UDP command socket." << std::endl;
        return;
    }

    const int reuse_enabled = 1;
    ::setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse_enabled, sizeof(reuse_enabled));

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    ::setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in address{};
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(listen_port);

    const int parse_result = ::inet_pton(AF_INET, bind_ip.c_str(), &(address.sin_addr));

    if (parse_result != 1) {
        std::cerr << "Invalid UDP bind IP: " << bind_ip << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Failed to bind UDP command socket." << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    sockaddr_in bound_address{};
    socklen_t bound_address_length = sizeof(bound_address);

    if (::getsockname(
            socket_fd_,
            reinterpret_cast<sockaddr*>(&bound_address),
            &bound_address_length
        ) < 0) {
        std::cerr << "Failed to read UDP command socket bound port." << std::endl;
        ::close(socket_fd_);
        socket_fd_ = -1;
        return;
    }

    bound_port_ = ntohs(bound_address.sin_port);
    ready_ = true;
}

UdpCommandReceiver::~UdpCommandReceiver() {
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
    }
}

bool UdpCommandReceiver::is_ready() const {
    return ready_;
}

std::uint16_t UdpCommandReceiver::bound_port() const {
    return bound_port_;
}

bool UdpCommandReceiver::receive_packet(CommandPacket& packet_out) const {
    if (!ready_ || socket_fd_ < 0) {
        return false;
    }

    std::array<std::uint8_t, 256> buffer{};

    const auto received = ::recvfrom(
        socket_fd_,
        buffer.data(),
        buffer.size(),
        0,
        nullptr,
        nullptr
    );

    if (received <= 0) {
        return false;
    }

    const std::vector<std::uint8_t> bytes(buffer.begin(), buffer.begin() + received);
    return deserialize_command_packet(bytes, packet_out);
}

}  // namespace astra
