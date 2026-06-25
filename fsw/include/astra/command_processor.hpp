#pragma once

#include "astra/command_packet.hpp"
#include "astra/mode_manager.hpp"

#include <cstdint>
#include <string>

namespace astra {

enum class CommandStatus : std::uint8_t {
    ACCEPTED = 0,
    REJECTED_BAD_ARGUMENT = 1,
    REJECTED_INVALID_TRANSITION = 2,
    REJECTED_UNKNOWN_COMMAND = 3
};

struct CommandResult {
    CommandStatus status = CommandStatus::ACCEPTED;
    CommandId command_id = CommandId::NOOP;
    std::uint32_t sequence_number = 0;
    Mode resulting_mode = Mode::BOOT;
    FaultCode resulting_fault = FaultCode::NONE;
    std::string message;
};

std::string command_status_to_string(CommandStatus status);

class CommandProcessor {
public:
    CommandProcessor(ModeManager& mode_manager);

    CommandResult process(const CommandPacket& packet);

    FaultCode last_fault() const;
    void clear_fault();

private:
    ModeManager& mode_manager_;
    FaultCode last_fault_;
};

}  // namespace astra
