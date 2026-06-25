#include "astra/telemetry_packet.hpp"
#include "astra/udp_telemetry_sender.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect_true(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        std::cout << "[FAIL] " << test_name << std::endl;
        ++failures;
    }
}

bool nearly_equal(float a, float b) {
    return std::fabs(a - b) < 0.0001F;
}

class UdpReceiverFixture {
public:
    UdpReceiverFixture() : socket_fd_(-1), port_(0), ready_(false) {
        socket_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);

        if (socket_fd_ < 0) {
            return;
        }

        timeval timeout{};
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        ::setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        sockaddr_in address{};
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;

        if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
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
            ::close(socket_fd_);
            socket_fd_ = -1;
            return;
        }

        port_ = ntohs(bound_address.sin_port);
        ready_ = true;
    }

    ~UdpReceiverFixture() {
        if (socket_fd_ >= 0) {
            ::close(socket_fd_);
        }
    }

    bool ready() const {
        return ready_;
    }

    std::uint16_t port() const {
        return port_;
    }

    bool receive(std::vector<std::uint8_t>& bytes_out) const {
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

        bytes_out.assign(buffer.begin(), buffer.begin() + received);
        return true;
    }

private:
    int socket_fd_;
    std::uint16_t port_;
    bool ready_;
};

void test_sender_initializes_with_valid_destination() {
    astra::UdpTelemetrySender sender("127.0.0.1", 5005);
    expect_true(sender.is_ready(), "UDP sender initializes with valid destination");
}

void test_sender_rejects_invalid_ip() {
    astra::UdpTelemetrySender sender("not_an_ip_address", 5005);
    expect_true(!sender.is_ready(), "UDP sender rejects invalid IP");
}

void test_sender_transmits_deserializable_packet() {
    UdpReceiverFixture receiver;
    expect_true(receiver.ready(), "UDP receiver fixture starts");

    if (!receiver.ready()) {
        return;
    }

    astra::UdpTelemetrySender sender("127.0.0.1", receiver.port());
    expect_true(sender.is_ready(), "UDP sender starts for loopback test");

    astra::TelemetryPacket packet;
    packet.sequence_number = 123;
    packet.timestamp_ms = 456789ULL;
    packet.mode = astra::Mode::NOMINAL;
    packet.last_fault = astra::FaultCode::NONE;
    packet.cpu_load_percent = 12.5F;
    packet.memory_load_percent = 34.75F;
    packet.heartbeat_count = 99;

    const bool sent = sender.send_packet(packet);
    expect_true(sent, "UDP sender reports packet sent");

    std::vector<std::uint8_t> received_bytes;
    const bool received = receiver.receive(received_bytes);
    expect_true(received, "UDP receiver gets packet");

    if (!received) {
        return;
    }

    astra::TelemetryPacket decoded;
    const bool decoded_ok = astra::deserialize_telemetry_packet(received_bytes, decoded);

    expect_true(decoded_ok, "received UDP payload deserializes");
    expect_true(decoded.sequence_number == 123, "received sequence number matches");
    expect_true(decoded.timestamp_ms == 456789ULL, "received timestamp matches");
    expect_true(decoded.mode == astra::Mode::NOMINAL, "received mode matches");
    expect_true(decoded.last_fault == astra::FaultCode::NONE, "received fault matches");
    expect_true(nearly_equal(decoded.cpu_load_percent, 12.5F), "received CPU load matches");
    expect_true(nearly_equal(decoded.memory_load_percent, 34.75F), "received memory load matches");
    expect_true(decoded.heartbeat_count == 99, "received heartbeat count matches");
}

}  // namespace

int main() {
    std::cout << "Running UdpTelemetrySender tests..." << std::endl;

    test_sender_initializes_with_valid_destination();
    test_sender_rejects_invalid_ip();
    test_sender_transmits_deserializable_packet();

    if (failures == 0) {
        std::cout << "All UdpTelemetrySender tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " UdpTelemetrySender test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
