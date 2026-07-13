#include "astra/command_processor.hpp"

#include <string>

namespace astra {

std::string command_status_to_string(CommandStatus status) {
    switch (status) {
        case CommandStatus::ACCEPTED:
            return "ACCEPTED";
        case CommandStatus::REJECTED_BAD_ARGUMENT:
            return "REJECTED_BAD_ARGUMENT";
        case CommandStatus::REJECTED_INVALID_TRANSITION:
            return "REJECTED_INVALID_TRANSITION";
        case CommandStatus::REJECTED_UNKNOWN_COMMAND:
            return "REJECTED_UNKNOWN_COMMAND";
        case CommandStatus::REJECTED_DUPLICATE_SEQUENCE:
            return "REJECTED_DUPLICATE_SEQUENCE";
        case CommandStatus::REJECTED_REPLAYED_SEQUENCE:
            return "REJECTED_REPLAYED_SEQUENCE";
        case CommandStatus::REJECTED_STALE_TIMESTAMP:
            return "REJECTED_STALE_TIMESTAMP";
        case CommandStatus::REJECTED_FUTURE_TIMESTAMP:
            return "REJECTED_FUTURE_TIMESTAMP";
        case CommandStatus::REJECTED_GUARD_CONFIGURATION:
            return "REJECTED_GUARD_CONFIGURATION";
        case CommandStatus::REJECTED_UNAUTHORIZED:
            return "REJECTED_UNAUTHORIZED";
        case CommandStatus::REJECTED_RECOVERY_LIMIT:
            return "REJECTED_RECOVERY_LIMIT";
    }

    return "UNKNOWN_STATUS";
}

CommandProcessor::CommandProcessor(
    ModeManager& mode_manager,
    RecoverySupervisorConfig recovery_configuration
)
    : mode_manager_(mode_manager),
      fdir_manager_(),
      recovery_supervisor_(recovery_configuration),
      last_fault_(FaultCode::NONE) {
    if (!recovery_supervisor_.valid()) {
        validation_error_ = recovery_supervisor_.validation_error();
        return;
    }

    valid_ = true;
}

bool CommandProcessor::valid() const {
    return valid_;
}

const std::string& CommandProcessor::validation_error() const {
    return validation_error_;
}

CommandResult CommandProcessor::process(const CommandPacket& packet) {
    CommandResult result;
    result.command_id = packet.command_id;
    result.sequence_number = packet.sequence_number;
    result.resulting_mode = mode_manager_.current_mode();
    result.resulting_fault = last_fault_;

    switch (packet.command_id) {
        case CommandId::NOOP:
            result.status = CommandStatus::ACCEPTED;
            result.message = "NOOP accepted";
            break;

        case CommandId::REQUEST_TELEMETRY:
            result.status = CommandStatus::ACCEPTED;
            result.message = "Telemetry request accepted";
            break;

        case CommandId::CLEAR_FAULT:
            last_fault_ = FaultCode::NONE;
            result.status = CommandStatus::ACCEPTED;
            result.resulting_fault = last_fault_;
            result.message = "Fault state cleared";
            break;

        case CommandId::SET_MODE: {
            Mode requested_mode = Mode::BOOT;

            if (!command_argument_to_mode(packet.argument, requested_mode)) {
                result.status = CommandStatus::REJECTED_BAD_ARGUMENT;
                result.message = "SET_MODE rejected: invalid mode argument";
                break;
            }

            const Mode mode_before = mode_manager_.current_mode();
            const bool transitioned = mode_manager_.transition_to(requested_mode, last_fault_);

            if (!transitioned) {
                if (mode_before == Mode::RECOVERY && requested_mode != Mode::RECOVERY) {
                    const auto recovery_result =
                        recovery_supervisor_.record_failure(mode_manager_);
                    if (recovery_result.fail_safe_forced) {
                        result.status = CommandStatus::REJECTED_RECOVERY_LIMIT;
                        result.message =
                            "SET_MODE rejected: recovery failure limit reached attempts=" +
                            std::to_string(recovery_result.failed_attempts) +
                            " fail_safe=SAFE";
                        break;
                    }
                }

                result.status = CommandStatus::REJECTED_INVALID_TRANSITION;
                result.message = "SET_MODE rejected: invalid mode transition";
                break;
            }

            if (mode_before == Mode::SAFE && requested_mode == Mode::RECOVERY) {
                recovery_supervisor_.begin_session();
            } else if (mode_before == Mode::RECOVERY &&
                       requested_mode != Mode::RECOVERY) {
                static_cast<void>(recovery_supervisor_.record_success());
            }

            result.status = CommandStatus::ACCEPTED;
            result.resulting_mode = mode_manager_.current_mode();
            result.message = "SET_MODE accepted";
            break;
        }

        case CommandId::INJECT_FAULT: {
            FaultCode requested_fault = FaultCode::NONE;

            if (!command_argument_to_fault(packet.argument, requested_fault)) {
                result.status = CommandStatus::REJECTED_BAD_ARGUMENT;
                result.message = "INJECT_FAULT rejected: invalid fault argument";
                break;
            }

            last_fault_ = requested_fault;
            const auto action = fdir_manager_.apply_fault(mode_manager_, requested_fault);

            result.status = CommandStatus::ACCEPTED;
            result.resulting_mode = mode_manager_.current_mode();
            result.resulting_fault = last_fault_;
            result.message =
                "INJECT_FAULT accepted: severity=" +
                fault_severity_to_string(action.disposition.severity) +
                " response=" +
                fault_response_to_string(action.disposition.response);
            if (action.safe_fallback_used) {
                result.message += " fallback=ENTER_SAFE";
            }
            break;
        }
    }

    result.resulting_mode = mode_manager_.current_mode();
    result.resulting_fault = last_fault_;
    return result;
}

FaultCode CommandProcessor::last_fault() const {
    return last_fault_;
}

void CommandProcessor::clear_fault() {
    last_fault_ = FaultCode::NONE;
}

}  // namespace astra
