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

bool nearly_equal(float a, float b) {
    return std::fabs(a - b) < 0.0001F;
}

void test_packet_has_fixed_size() {
    astra::TelemetryPacket packet;
    const auto bytes = astra::serialize_telemetry_packet(packet);

    expect_true(
        bytes.size() == astra::TELEMETRY_PACKET_SIZE_BYTES,
        "serialized packet has fixed expected size"
    );
}

void test_telemetry_round_trip() {
    astra::TelemetryPacket packet;
    packet.sequence_number = 42;
    packet.timestamp_ms = 123456789ULL;
    packet.mode = astra::Mode::DEGRADED_PAYLOAD;
    packet.last_fault = astra::FaultCode::CPU_OVERLOAD;
    packet.cpu_load_percent = 87.5F;
    packet.memory_load_percent = 64.25F;
    packet.heartbeat_count = 9001;

    const auto bytes = astra::serialize_telemetry_packet(packet);

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(ok, "valid telemetry packet deserializes");
    expect_true(decoded.sequence_number == 42, "sequence number round trip");
    expect_true(decoded.timestamp_ms == 123456789ULL, "timestamp round trip");
    expect_true(decoded.mode == astra::Mode::DEGRADED_PAYLOAD, "mode round trip");
    expect_true(decoded.last_fault == astra::FaultCode::CPU_OVERLOAD, "fault code round trip");
    expect_true(nearly_equal(decoded.cpu_load_percent, 87.5F), "CPU load round trip");
    expect_true(nearly_equal(decoded.memory_load_percent, 64.25F), "memory load round trip");
    expect_true(decoded.heartbeat_count == 9001, "heartbeat count round trip");
}

void test_corrupted_packet_rejected() {
    astra::TelemetryPacket packet;
    packet.sequence_number = 7;

    auto bytes = astra::serialize_telemetry_packet(packet);
    bytes.at(10) ^= 0xFFU;

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "corrupted packet rejected by CRC");
}

void test_wrong_size_packet_rejected() {
    astra::TelemetryPacket packet;
    auto bytes = astra::serialize_telemetry_packet(packet);
    bytes.pop_back();

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "wrong-size packet rejected");
}

void test_invalid_mode_rejected() {
    astra::TelemetryPacket packet;
    auto bytes = astra::serialize_telemetry_packet(packet);

    constexpr std::size_t mode_offset = 4 + 2 + 2 + 4 + 8;
    bytes.at(mode_offset) = 99;

    const std::vector<std::uint8_t> without_crc(bytes.begin(), bytes.end() - 2);
    const std::uint16_t crc = astra::telemetry_crc16_ccitt(without_crc);
    bytes.at(bytes.size() - 2) = static_cast<std::uint8_t>(crc & 0xFFU);
    bytes.at(bytes.size() - 1) = static_cast<std::uint8_t>((crc >> 8U) & 0xFFU);

    astra::TelemetryPacket decoded;
    const bool ok = astra::deserialize_telemetry_packet(bytes, decoded);

    expect_true(!ok, "invalid mode rejected even with valid CRC");
}

}  // namespace

int main() {
    std::cout << "Running TelemetryPacket tests..." << std::endl;

    test_packet_has_fixed_size();
    test_telemetry_round_trip();
    test_corrupted_packet_rejected();
    test_wrong_size_packet_rejected();
    test_invalid_mode_rejected();

    if (failures == 0) {
        std::cout << "All TelemetryPacket tests passed." << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << failures << " TelemetryPacket test(s) failed." << std::endl;
    return EXIT_FAILURE;
}
