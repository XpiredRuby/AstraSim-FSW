#pragma once

#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace astra {

constexpr std::uint32_t COMMAND_MAGIC = 0x434D4453;  // ASCII: CMDS
constexpr std::uint16_t COMMAND_VERSION = 1;
constexpr std::size_t COMMAND_PACKET_SIZE_BYTES = 26;

enum class CommandId : std::uint16_t {
    NOOP = 0,
    SET_MODE = 1,
    INJECT_FAULT = 2,
    REQUEST_TELEMETRY = 3,
    CLEAR_FAULT = 4
};

struct CommandPacket {
    std::uint32_t sequence_number = 0;
    std::uint64_t timestamp_ms = 0;

    CommandId command_id = CommandId::NOOP;

    // Meaning depends on command_id:
    // SET_MODE: desired Mode value
    // INJECT_FAULT: desired FaultCode value
    // REQUEST_TELEMETRY / CLEAR_FAULT / NOOP: unused, set to 0
    std::uint32_t argument = 0;
};

std::string command_id_to_string(CommandId command_id);

std::uint16_t command_crc16_ccitt(const std::vector<std::uint8_t>& bytes);

std::vector<std::uint8_t> serialize_command_packet(const CommandPacket& packet);

bool deserialize_command_packet(
    const std::vector<std::uint8_t>& bytes,
    CommandPacket& packet_out
);

bool command_argument_to_mode(std::uint32_t argument, Mode& mode_out);

bool command_argument_to_fault(std::uint32_t argument, FaultCode& fault_out);

}  // namespace astra
