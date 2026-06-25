#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <vector>

namespace astra {

constexpr std::uint32_t TELEMETRY_MAGIC = 0x41535452;  // ASCII: ASTR
constexpr std::uint16_t TELEMETRY_VERSION = 1;
constexpr std::uint16_t TELEMETRY_PACKET_TYPE_HEALTH = 1;
constexpr std::size_t TELEMETRY_PACKET_SIZE_BYTES = 37;

struct TelemetryPacket {
    std::uint32_t sequence_number = 0;
    std::uint64_t timestamp_ms = 0;

    Mode mode = Mode::BOOT;
    FaultCode last_fault = FaultCode::NONE;

    float cpu_load_percent = 0.0F;
    float memory_load_percent = 0.0F;

    std::uint32_t heartbeat_count = 0;
};

std::uint16_t telemetry_crc16_ccitt(const std::vector<std::uint8_t>& bytes);

std::vector<std::uint8_t> serialize_telemetry_packet(const TelemetryPacket& packet);

bool deserialize_telemetry_packet(
    const std::vector<std::uint8_t>& bytes,
    TelemetryPacket& packet_out
);

}  // namespace astra
