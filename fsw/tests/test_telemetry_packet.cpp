#include "astra/telemetry_packet.hpp"

#include <cmath>
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

astra::TelemetryPacket make_packet() {
    astra::TelemetryPacket packet;
    packet.sequence_number = 42;
    packet.timestamp_ms = 123456789ULL;
    packet.mode = astra::Mode::DEGRADED_PAYLOAD;
    packet.last_fault = astra::FaultCode::CPU_OVERLOAD;
    packet.cpu_load_percent = 76.5F;
    packet.memory_load_percent = 52.25F;
    packet.heartbeat_count = 99;
    packet.last_command_sequence_number = 7;
    packet.last_command_id = 2;
    packet.last_command_status = 0;
    return packet;
}

void test_serialized_packet_size() {
    const auto bytes = astra::serialize_telemetry_packet(make_packet());

    expect_true(
        bytes.size() == astra::TELEMETRY_PACKET_SIZE_BYTES,
        "serialized telemetry packet has expected size"
    );
    expect_true(
        astra::TELEMETRY_PACKET_SIZE_BYTES == 43,
        "telemetry packet size includes command ACK fields"
    );
}

void test_round_trip_serialization() {
    const auto original = make_packet();
    const auto bytes = astra::serialize_telemetry_packet(original);

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(ok, "valid telemetry packet deserializes");
    expect_true(decoded.sequence_number == original.sequence_number, "sequence number round trips");
    expect_true(decoded.timestamp_ms == original.timestamp_ms, "timestamp round trips");
    expect_true(decoded.mode == original.mode, "mode round trips");
    expect_true(decoded.last_fault == original.last_fault, "fault round trips");
    expect_true(
        std::fabs(decoded.cpu_load_percent - original.cpu_load_percent) < 0.001F,
        "CPU load round trips"
    );
    expect_true(
        std::fabs(decoded.memory_load_percent - original.memory_load_percent) < 0.001F,
        "memory load round trips"
    );
    expect_true(decoded.heartbeat_count == original.heartbeat_count, "heartbeat count round trips");
    expect_true(
        decoded.last_command_sequence_number == original.last_command_sequence_number,
        "last command sequence number round trips"
    );
    expect_true(decoded.last_command_id == original.last_command_id, "last command ID round trips");
    expect_true(
        decoded.last_command_status == original.last_command_status,
        "last command status round trips"
    );
}

void test_bad_crc_is_rejected() {
    auto bytes = astra::serialize_telemetry_packet(make_packet());
    bytes.at(10) ^= 0xFFU;

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "packet with bad CRC is rejected");
}

void test_wrong_size_is_rejected() {
    auto bytes = astra::serialize_telemetry_packet(make_packet());
    bytes.pop_back();

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "wrong packet size is rejected");
}

void test_invalid_mode_is_rejected() {
    auto bytes = astra::serialize_telemetry_packet(make_packet());

    constexpr std::size_t mode_offset = 20;
    bytes.at(mode_offset) = 99;

    const std::vector<std::uint8_t> without_crc(bytes.begin(), bytes.end() - 2);
    const std::uint16_t crc = astra::telemetry_crc16_ccitt(without_crc);
    bytes.at(bytes.size() - 2) = static_cast<std::uint8_t>(crc & 0xFFU);
    bytes.at(bytes.size() - 1) = static_cast<std::uint8_t>((crc >> 8U) & 0xFFU);

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "invalid mode is rejected");
}

}  // namespace

int main() {
    std::cout << "Running telemetry packet tests..." << std::endl;

    test_serialized_packet_size();
    test_round_trip_serialization();
    test_bad_crc_is_rejected();
    test_wrong_size_is_rejected();
    test_invalid_mode_is_rejected();

    if (failures == 0) {
        std::cout << "All telemetry packet tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " telemetry packet test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
