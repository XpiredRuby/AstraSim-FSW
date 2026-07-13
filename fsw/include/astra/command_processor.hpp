#pragma once

#include "astra/command_packet.hpp"
#include "astra/fdir_manager.hpp"
#include "astra/mode_manager.hpp"
#include "astra/recovery_supervisor.hpp"

#include <cstdint>
#include <string>

namespace astra {

enum class CommandStatus : std::uint8_t {
    ACCEPTED = 0,
    REJECTED_BAD_ARGUMENT = 1,
    REJECTED_INVALID_TRANSITION = 2,
    REJECTED_UNKNOWN_COMMAND = 3,
    REJECTED_DUPLICATE_SEQUENCE = 4,
    REJECTED_REPLAYED_SEQUENCE = 5,
    REJECTED_STALE_TIMESTAMP = 6,
    REJECTED_FUTURE_TIMESTAMP = 7,
    REJECTED_GUARD_CONFIGURATION = 8,
    REJECTED_UNAUTHORIZED = 9,
    REJECTED_RECOVERY_LIMIT = 10
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
    explicit CommandProcessor(
        ModeManager& mode_manager,
        RecoverySupervisorConfig recovery_configuration = RecoverySupervisorConfig{}
    );

    bool valid() const;
    const std::string& validation_error() const;

    CommandResult process(const CommandPacket& packet);

    FaultCode last_fault() const;
    void clear_fault();

private:
    ModeManager& mode_manager_;
    FdirManager fdir_manager_;
    RecoverySupervisor recovery_supervisor_;
    FaultCode last_fault_;
    bool valid_ = false;
    std::string validation_error_;
};

}  // namespace astra
