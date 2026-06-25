#include "astra/command_packet.hpp"
#include "astra/udp_command_receiver.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

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

bool send_bytes_to_loopback(std::uint16_t port, const std::vector<std::uint8_t>& bytes) {
    const int socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        return false;
    }

    sockaddr_in destination{};
    std::memset(&destination, 0, sizeof(destination));
    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);

    if (::inet_pton(AF_INET, "127.0.0.1", &(destination.sin_addr)) != 1) {
        ::close(socket_fd);
        return false;
    }

    const auto sent = ::sendto(
        socket_fd,
        bytes.data(),
        bytes.size(),
        0,
        reinterpret_cast<const sockaddr*>(&destination),
        sizeof(destination)
    );

    ::close(socket_fd);
    return sent == static_cast<ssize_t>(bytes.size());
}

void test_receiver_initializes() {
    astra::UdpCommandReceiver receiver(0, "127.0.0.1", 500);

    expect_true(receiver.is_ready(), "UDP command receiver initializes");
    expect_true(receiver.bound_port() != 0, "UDP command receiver binds an ephemeral port");
}

void test_receiver_rejects_invalid_bind_ip() {
    astra::UdpCommandReceiver receiver(0, "not_an_ip_address", 500);
    expect_true(!receiver.is_ready(), "UDP command receiver rejects invalid bind IP");
}

void test_receiver_accepts_valid_command_packet() {
    astra::UdpCommandReceiver receiver(0, "127.0.0.1", 1000);
    expect_true(receiver.is_ready(), "UDP command receiver ready for valid packet test");

    if (!receiver.is_ready()) {
        return;
    }

    astra::CommandPacket command;
    command.sequence_number = 77;
    command.timestamp_ms = 123456ULL;
    command.command_id = astra::CommandId::SET_MODE;
    command.argument = static_cast<std::uint32_t>(astra::Mode::NOMINAL);

    const auto bytes = astra::serialize_command_packet(command);
    expect_true(send_bytes_to_loopback(receiver.bound_port(), bytes), "test command sent to receiver");

    astra::CommandPacket received;
    const bool ok = receiver.receive_packet(received);

    expect_true(ok, "receiver decodes valid command");
    expect_true(received.sequence_number == 77, "received command sequence matches");
    expect_true(received.timestamp_ms == 123456ULL, "received command timestamp matches");
    expect_true(received.command_id == astra::CommandId::SET_MODE, "received command ID matches");
    expect_true(
        received.argument == static_cast<std::uint32_t>(astra::Mode::NOMINAL),
        "received command argument matches"
    );
}

void test_receiver_rejects_corrupted_command_packet() {
    astra::UdpCommandReceiver receiver(0, "127.0.0.1", 1000);
    expect_true(receiver.is_ready(), "UDP command receiver ready for corrupted packet test");

    if (!receiver.is_ready()) {
        return;
    }

    astra::CommandPacket command;
    command.sequence_number = 88;
    command.command_id = astra::CommandId::INJECT_FAULT;
    command.argument = static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD);

    auto bytes = astra::serialize_command_packet(command);
    bytes.at(10) ^= 0xFFU;

    expect_true(send_bytes_to_loopback(receiver.bound_port(), bytes), "corrupted command sent to receiver");

    astra::CommandPacket received;
    const bool ok = receiver.receive_packet(received);

    expect_true(!ok, "receiver rejects corrupted command");
}

}  // namespace

int main() {
    std::cout << "Running UdpCommandReceiver tests..." << std::endl;

    test_receiver_initializes();
    test_receiver_rejects_invalid_bind_ip();
    test_receiver_accepts_valid_command_packet();
    test_receiver_rejects_corrupted_command_packet();

    if (failures == 0) {
        std::cout << "All UdpCommandReceiver tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " UdpCommandReceiver test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
