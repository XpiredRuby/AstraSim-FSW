#include "astra/command_packet.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace {

[[noreturn]] void invariant_failure() {
    std::abort();
}

bool packets_equal(
    const astra::CommandPacket& left,
    const astra::CommandPacket& right
) {
    return left.sequence_number == right.sequence_number &&
           left.timestamp_ms == right.timestamp_ms &&
           left.command_id == right.command_id &&
           left.argument == right.argument;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(
    const std::uint8_t* data,
    std::size_t size
) {
    const std::vector<std::uint8_t> bytes(data, data + size);

    astra::CommandPacket decoded;
    decoded.sequence_number = 0xA5A5A5A5U;
    decoded.timestamp_ms = 0x0102030405060708ULL;
    decoded.command_id = astra::CommandId::CLEAR_FAULT;
    decoded.argument = 0x5A5A5A5AU;
    const astra::CommandPacket sentinel = decoded;

    const bool accepted = astra::deserialize_command_packet(bytes, decoded);

    if (!accepted) {
        if (!packets_equal(decoded, sentinel)) {
            invariant_failure();
        }
        return 0;
    }

    if (bytes.size() != astra::COMMAND_PACKET_SIZE_BYTES) {
        invariant_failure();
    }

    const auto canonical_bytes = astra::serialize_command_packet(decoded);
    if (canonical_bytes.size() != astra::COMMAND_PACKET_SIZE_BYTES) {
        invariant_failure();
    }

    astra::CommandPacket round_trip;
    if (!astra::deserialize_command_packet(canonical_bytes, round_trip)) {
        invariant_failure();
    }

    if (!packets_equal(decoded, round_trip)) {
        invariant_failure();
    }

    astra::Mode mode = astra::Mode::BOOT;
    astra::FaultCode fault = astra::FaultCode::NONE;

    switch (decoded.command_id) {
        case astra::CommandId::SET_MODE:
            static_cast<void>(astra::command_argument_to_mode(decoded.argument, mode));
            break;
        case astra::CommandId::INJECT_FAULT:
            static_cast<void>(astra::command_argument_to_fault(decoded.argument, fault));
            break;
        case astra::CommandId::NOOP:
        case astra::CommandId::REQUEST_TELEMETRY:
        case astra::CommandId::CLEAR_FAULT:
            break;
    }

    return 0;
}
