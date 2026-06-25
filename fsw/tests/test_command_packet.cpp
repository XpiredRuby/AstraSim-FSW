#include "astra/command_packet.hpp"

#include <cstdlib>
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

void test_command_packet_has_fixed_size() {
    astra::CommandPacket packet;
    const auto bytes = astra::serialize_command_packet(packet);

    expect_true(
        bytes.size() == astra::COMMAND_PACKET_SIZE_BYTES,
        "serialized command packet has fixed expected size"
    );
}

void test_command_packet_round_trip_set_mode() {
    astra::CommandPacket packet;
    packet.sequence_number = 42;
    packet.timestamp_ms = 123456789ULL;
    packet.command_id = astra::CommandId::SET_MODE;
    packet.argument = static_cast<std::uint32_t>(astra::Mode::SAFE);

    const auto bytes = astra::serialize_command_packet(packet);

    astra::CommandPacket decoded;
    const bool ok = astra::deserialize_command_packet(bytes, decoded);

    expect_true(ok, "valid SET_MODE command deserializes");
    expect_true(decoded.sequence_number == 42, "sequence number round trip");
    expect_true(decoded.timestamp_ms == 123456789ULL, "timestamp round trip");
    expect_true(decoded.command_id == astra::CommandId::SET_MODE, "command id round trip");
    expect_true(decoded.argument == static_cast<std::uint32_t>(astra::Mode::SAFE), "argument round trip");
}

void test_command_packet_round_trip_inject_fault() {
    astra::CommandPacket packet;
    packet.sequence_number = 9;
    packet.timestamp_ms = 987654321ULL;
    packet.command_id = astra::CommandId::INJECT_FAULT;
    packet.argument = static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD);

    const auto bytes = astra::serialize_command_packet(packet);

    astra::CommandPacket decoded;
    const bool ok = astra::deserialize_command_packet(bytes, decoded);

    expect_true(ok, "valid INJECT_FAULT command deserializes");
    expect_true(decoded.command_id == astra::CommandId::INJECT_FAULT, "inject fault command id round trip");
    expect_true(
        decoded.argument == static_cast<std::uint32_t>(astra::FaultCode::CPU_OVERLOAD),
        "inject fault argument round trip"
    );
}

void test_corrupted_command_rejected() {
    astra::CommandPacket packet;
    packet.sequence_number = 7;
    packet.command_id = astra::CommandId::REQUEST_TELEMETRY;

    auto bytes = astra::serialize_command_packet(packet);
    bytes.at(10) ^= 0xFFU;

    astra::CommandPacket decoded;
    const bool ok = astra::deserialize_command_packet(bytes, decoded);

    expect_true(!ok, "corrupted command rejected by CRC");
}

void test_wrong_size_command_rejected() {
    astra::CommandPacket packet;
    auto bytes = astra::serialize_command_packet(packet);
    bytes.pop_back();

    astra::CommandPacket decoded;
    const bool ok = astra::deserialize_command_packet(bytes, decoded);

    expect_true(!ok, "wrong-size command rejected");
}

void test_invalid_command_id_rejected() {
    astra::CommandPacket packet;
    auto bytes = astra::serialize_command_packet(packet);

    constexpr std::size_t command_id_offset = 4 + 2 + 4 + 8;
    bytes.at(command_id_offset) = 99;
    bytes.at(command_id_offset + 1) = 0;

    const std::vector<std::uint8_t> without_crc(bytes.begin(), bytes.end() - 2);
    const std::uint16_t crc = astra::command_crc16_ccitt(without_crc);
    bytes.at(bytes.size() - 2) = static_cast<std::uint8_t>(crc & 0xFFU);
    bytes.at(bytes.size() - 1) = static_cast<std::uint8_t>((crc >> 8U) & 0xFFU);

    astra::CommandPacket decoded;
    const bool ok = astra::deserialize_command_packet(bytes, decoded);

    expect_true(!ok, "invalid command id rejected even with valid CRC");
}

void test_command_argument_to_mode() {
    astra::Mode mode = astra::Mode::BOOT;

    const bool ok = astra::command_argument_to_mode(
        static_cast<std::uint32_t>(astra::Mode::RECOVERY),
        mode
    );

    expect_true(ok, "valid mode argument accepted");
    expect_true(mode == astra::Mode::RECOVERY, "mode argument decoded");

    const bool bad = astra::command_argument_to_mode(99, mode);
    expect_true(!bad, "invalid mode argument rejected");
}

void test_command_argument_to_fault() {
    astra::FaultCode fault = astra::FaultCode::NONE;

    const bool ok = astra::command_argument_to_fault(
        static_cast<std::uint32_t>(astra::FaultCode::SENSOR_TIMEOUT),
        fault
    );

    expect_true(ok, "valid fault argument accepted");
    expect_true(fault == astra::FaultCode::SENSOR_TIMEOUT, "fault argument decoded");

    const bool bad = astra::command_argument_to_fault(9999, fault);
    expect_true(!bad, "invalid fault argument rejected");
}

void test_command_id_string_names() {
    expect_true(
        astra::command_id_to_string(astra::CommandId::INJECT_FAULT) == "INJECT_FAULT",
        "command id string conversion works"
    );
}

}  // namespace

int main() {
    std::cout << "Running CommandPacket tests..." << std::endl;

    test_command_packet_has_fixed_size();
    test_command_packet_round_trip_set_mode();
    test_command_packet_round_trip_inject_fault();
    test_corrupted_command_rejected();
    test_wrong_size_command_rejected();
    test_invalid_command_id_rejected();
    test_command_argument_to_mode();
    test_command_argument_to_fault();
    test_command_id_string_names();

    if (failures == 0) {
        std::cout << "All CommandPacket tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " CommandPacket test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
