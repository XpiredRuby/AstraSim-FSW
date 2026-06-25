#include "astra/telemetry_packet.hpp"

#include <cstring>

namespace astra {
namespace {

void append_u8(std::vector<std::uint8_t>& bytes, std::uint8_t value) {
    bytes.push_back(value);
}

void append_u16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void append_u32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 24U) & 0xFFU));
}

void append_u64(std::vector<std::uint8_t>& bytes, std::uint64_t value) {
    for (int shift = 0; shift < 64; shift += 8) {
        bytes.push_back(static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
}

void append_float32(std::vector<std::uint8_t>& bytes, float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t), "float must be 32 bits");

    std::uint32_t raw = 0;
    std::memcpy(&raw, &value, sizeof(float));
    append_u32(bytes, raw);
}

std::uint8_t read_u8(const std::vector<std::uint8_t>& bytes, std::size_t& offset) {
    const std::uint8_t value = bytes.at(offset);
    offset += 1;
    return value;
}

std::uint16_t read_u16(const std::vector<std::uint8_t>& bytes, std::size_t& offset) {
    const std::uint16_t value =
        static_cast<std::uint16_t>(bytes.at(offset)) |
        (static_cast<std::uint16_t>(bytes.at(offset + 1)) << 8U);

    offset += 2;
    return value;
}

std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes, std::size_t& offset) {
    const std::uint32_t value =
        static_cast<std::uint32_t>(bytes.at(offset)) |
        (static_cast<std::uint32_t>(bytes.at(offset + 1)) << 8U) |
        (static_cast<std::uint32_t>(bytes.at(offset + 2)) << 16U) |
        (static_cast<std::uint32_t>(bytes.at(offset + 3)) << 24U);

    offset += 4;
    return value;
}

std::uint64_t read_u64(const std::vector<std::uint8_t>& bytes, std::size_t& offset) {
    std::uint64_t value = 0;

    for (int shift = 0; shift < 64; shift += 8) {
        value |= static_cast<std::uint64_t>(bytes.at(offset)) << shift;
        offset += 1;
    }

    return value;
}

float read_float32(const std::vector<std::uint8_t>& bytes, std::size_t& offset) {
    const std::uint32_t raw = read_u32(bytes, offset);

    float value = 0.0F;
    std::memcpy(&value, &raw, sizeof(float));
    return value;
}

bool is_valid_mode(std::uint8_t value) {
    switch (static_cast<Mode>(value)) {
        case Mode::BOOT:
        case Mode::NOMINAL:
        case Mode::DEGRADED_SENSOR:
        case Mode::DEGRADED_PAYLOAD:
        case Mode::SAFE:
        case Mode::RECOVERY:
            return true;
    }

    return false;
}

bool is_valid_fault_code(std::uint16_t value) {
    switch (static_cast<FaultCode>(value)) {
        case FaultCode::NONE:
        case FaultCode::SENSOR_TIMEOUT:
        case FaultCode::SENSOR_INVALID_DATA:
        case FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT:
        case FaultCode::CPU_OVERLOAD:
        case FaultCode::MEMORY_OVERLOAD:
        case FaultCode::TELEMETRY_SOCKET_FAILURE:
        case FaultCode::COMMAND_BAD_CRC:
        case FaultCode::COMMAND_UNKNOWN_ID:
        case FaultCode::COMMAND_TIMEOUT:
        case FaultCode::WATCHDOG_DEADLINE_MISS:
            return true;
    }

    return false;
}

}  // namespace

std::uint16_t telemetry_crc16_ccitt(const std::vector<std::uint8_t>& bytes) {
    std::uint16_t crc = 0xFFFFU;

    for (const auto byte : bytes) {
        crc ^= static_cast<std::uint16_t>(byte) << 8U;

        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x8000U) != 0U) {
                crc = static_cast<std::uint16_t>((crc << 1U) ^ 0x1021U);
            } else {
                crc = static_cast<std::uint16_t>(crc << 1U);
            }
        }
    }

    return crc;
}

std::vector<std::uint8_t> serialize_telemetry_packet(const TelemetryPacket& packet) {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(TELEMETRY_PACKET_SIZE_BYTES);

    append_u32(bytes, TELEMETRY_MAGIC);
    append_u16(bytes, TELEMETRY_VERSION);
    append_u16(bytes, TELEMETRY_PACKET_TYPE_HEALTH);

    append_u32(bytes, packet.sequence_number);
    append_u64(bytes, packet.timestamp_ms);

    append_u8(bytes, static_cast<std::uint8_t>(packet.mode));
    append_u16(bytes, static_cast<std::uint16_t>(packet.last_fault));

    append_float32(bytes, packet.cpu_load_percent);
    append_float32(bytes, packet.memory_load_percent);

    append_u32(bytes, packet.heartbeat_count);

    const std::uint16_t crc = telemetry_crc16_ccitt(bytes);
    append_u16(bytes, crc);

    return bytes;
}

bool deserialize_telemetry_packet(
    const std::vector<std::uint8_t>& bytes,
    TelemetryPacket& packet_out
) {
    if (bytes.size() != TELEMETRY_PACKET_SIZE_BYTES) {
        return false;
    }

    const std::vector<std::uint8_t> without_crc(bytes.begin(), bytes.end() - 2);
    const std::uint16_t expected_crc = telemetry_crc16_ccitt(without_crc);

    std::size_t crc_offset = bytes.size() - 2;
    const std::uint16_t actual_crc = read_u16(bytes, crc_offset);

    if (actual_crc != expected_crc) {
        return false;
    }

    std::size_t offset = 0;

    const std::uint32_t magic = read_u32(bytes, offset);
    const std::uint16_t version = read_u16(bytes, offset);
    const std::uint16_t packet_type = read_u16(bytes, offset);

    if (magic != TELEMETRY_MAGIC) {
        return false;
    }

    if (version != TELEMETRY_VERSION) {
        return false;
    }

    if (packet_type != TELEMETRY_PACKET_TYPE_HEALTH) {
        return false;
    }

    TelemetryPacket packet;
    packet.sequence_number = read_u32(bytes, offset);
    packet.timestamp_ms = read_u64(bytes, offset);

    const std::uint8_t raw_mode = read_u8(bytes, offset);
    if (!is_valid_mode(raw_mode)) {
        return false;
    }
    packet.mode = static_cast<Mode>(raw_mode);

    const std::uint16_t raw_fault = read_u16(bytes, offset);
    if (!is_valid_fault_code(raw_fault)) {
        return false;
    }
    packet.last_fault = static_cast<FaultCode>(raw_fault);

    packet.cpu_load_percent = read_float32(bytes, offset);
    packet.memory_load_percent = read_float32(bytes, offset);
    packet.heartbeat_count = read_u32(bytes, offset);

    packet_out = packet;
    return true;
}

}  // namespace astra
