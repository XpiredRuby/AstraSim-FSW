#include "astra/command_packet.hpp"

namespace astra {
namespace {

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

bool is_valid_command_id(std::uint16_t value) {
    switch (static_cast<CommandId>(value)) {
        case CommandId::NOOP:
        case CommandId::SET_MODE:
        case CommandId::INJECT_FAULT:
        case CommandId::REQUEST_TELEMETRY:
        case CommandId::CLEAR_FAULT:
            return true;
    }

    return false;
}

}  // namespace

std::string command_id_to_string(CommandId command_id) {
    switch (command_id) {
        case CommandId::NOOP:
            return "NOOP";
        case CommandId::SET_MODE:
            return "SET_MODE";
        case CommandId::INJECT_FAULT:
            return "INJECT_FAULT";
        case CommandId::REQUEST_TELEMETRY:
            return "REQUEST_TELEMETRY";
        case CommandId::CLEAR_FAULT:
            return "CLEAR_FAULT";
    }

    return "UNKNOWN_COMMAND";
}

std::uint16_t command_crc16_ccitt(const std::vector<std::uint8_t>& bytes) {
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

std::vector<std::uint8_t> serialize_command_packet(const CommandPacket& packet) {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(COMMAND_PACKET_SIZE_BYTES);

    append_u32(bytes, COMMAND_MAGIC);
    append_u16(bytes, COMMAND_VERSION);

    append_u32(bytes, packet.sequence_number);
    append_u64(bytes, packet.timestamp_ms);

    append_u16(bytes, static_cast<std::uint16_t>(packet.command_id));
    append_u32(bytes, packet.argument);

    const std::uint16_t crc = command_crc16_ccitt(bytes);
    append_u16(bytes, crc);

    return bytes;
}

bool deserialize_command_packet(
    const std::vector<std::uint8_t>& bytes,
    CommandPacket& packet_out
) {
    if (bytes.size() != COMMAND_PACKET_SIZE_BYTES) {
        return false;
    }

    const std::vector<std::uint8_t> without_crc(bytes.begin(), bytes.end() - 2);
    const std::uint16_t expected_crc = command_crc16_ccitt(without_crc);

    std::size_t crc_offset = bytes.size() - 2;
    const std::uint16_t actual_crc = read_u16(bytes, crc_offset);

    if (actual_crc != expected_crc) {
        return false;
    }

    std::size_t offset = 0;

    const std::uint32_t magic = read_u32(bytes, offset);
    const std::uint16_t version = read_u16(bytes, offset);

    if (magic != COMMAND_MAGIC) {
        return false;
    }

    if (version != COMMAND_VERSION) {
        return false;
    }

    CommandPacket packet;
    packet.sequence_number = read_u32(bytes, offset);
    packet.timestamp_ms = read_u64(bytes, offset);

    const std::uint16_t raw_command_id = read_u16(bytes, offset);
    if (!is_valid_command_id(raw_command_id)) {
        return false;
    }

    packet.command_id = static_cast<CommandId>(raw_command_id);
    packet.argument = read_u32(bytes, offset);

    packet_out = packet;
    return true;
}

bool command_argument_to_mode(std::uint32_t argument, Mode& mode_out) {
    switch (static_cast<Mode>(argument)) {
        case Mode::BOOT:
        case Mode::NOMINAL:
        case Mode::DEGRADED_SENSOR:
        case Mode::DEGRADED_PAYLOAD:
        case Mode::SAFE:
        case Mode::RECOVERY:
        case Mode::STANDBY:
        case Mode::TEST:
            mode_out = static_cast<Mode>(argument);
            return true;
    }

    return false;
}

bool command_argument_to_fault(std::uint32_t argument, FaultCode& fault_out) {
    switch (static_cast<FaultCode>(argument)) {
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
            fault_out = static_cast<FaultCode>(argument);
            return true;
    }

    return false;
}

}  // namespace astra
