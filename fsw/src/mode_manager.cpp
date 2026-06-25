#include "astra/mode_manager.hpp"

namespace astra {

ModeManager::ModeManager()
    : mode_(Mode::BOOT) {}

Mode ModeManager::current_mode() const {
    return mode_;
}

bool ModeManager::transition_to(Mode next_mode, FaultCode /*reason*/) {
    if (!is_valid_transition(mode_, next_mode)) {
        return false;
    }

    mode_ = next_mode;
    return true;
}

void ModeManager::handle_fault(FaultCode fault) {
    switch (fault) {
        case FaultCode::SENSOR_TIMEOUT:
        case FaultCode::SENSOR_INVALID_DATA:
        case FaultCode::TELEMETRY_SOCKET_FAILURE:
        case FaultCode::WATCHDOG_DEADLINE_MISS:
            transition_to(Mode::SAFE, fault);
            break;

        case FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT:
        case FaultCode::CPU_OVERLOAD:
            transition_to(Mode::DEGRADED_PAYLOAD, fault);
            break;

        case FaultCode::NONE:
        default:
            break;
    }
}

bool ModeManager::is_valid_transition(Mode from, Mode to) const {
    if (from == to) {
        return true;
    }

    switch (from) {
        case Mode::BOOT:
            return to == Mode::NOMINAL || to == Mode::SAFE;

        case Mode::NOMINAL:
            return to == Mode::DEGRADED_SENSOR ||
                   to == Mode::DEGRADED_PAYLOAD ||
                   to == Mode::SAFE;

        case Mode::DEGRADED_SENSOR:
            return to == Mode::RECOVERY || to == Mode::SAFE;

        case Mode::DEGRADED_PAYLOAD:
            return to == Mode::NOMINAL ||
                   to == Mode::RECOVERY ||
                   to == Mode::SAFE;

        case Mode::SAFE:
            return to == Mode::RECOVERY;

        case Mode::RECOVERY:
            return to == Mode::NOMINAL || to == Mode::SAFE;

        default:
            return false;
    }
}

std::string mode_to_string(Mode mode) {
    switch (mode) {
        case Mode::BOOT: return "BOOT";
        case Mode::NOMINAL: return "NOMINAL";
        case Mode::DEGRADED_SENSOR: return "DEGRADED_SENSOR";
        case Mode::DEGRADED_PAYLOAD: return "DEGRADED_PAYLOAD";
        case Mode::SAFE: return "SAFE";
        case Mode::RECOVERY: return "RECOVERY";
        default: return "UNKNOWN";
    }
}

std::string fault_to_string(FaultCode fault) {
    switch (fault) {
        case FaultCode::NONE: return "NONE";
        case FaultCode::SENSOR_TIMEOUT: return "SENSOR_TIMEOUT";
        case FaultCode::SENSOR_INVALID_DATA: return "SENSOR_INVALID_DATA";
        case FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT: return "PAYLOAD_HEARTBEAT_TIMEOUT";
        case FaultCode::CPU_OVERLOAD: return "CPU_OVERLOAD";
        case FaultCode::MEMORY_OVERLOAD: return "MEMORY_OVERLOAD";
        case FaultCode::TELEMETRY_SOCKET_FAILURE: return "TELEMETRY_SOCKET_FAILURE";
        case FaultCode::COMMAND_BAD_CRC: return "COMMAND_BAD_CRC";
        case FaultCode::COMMAND_UNKNOWN_ID: return "COMMAND_UNKNOWN_ID";
        case FaultCode::COMMAND_TIMEOUT: return "COMMAND_TIMEOUT";
        case FaultCode::WATCHDOG_DEADLINE_MISS: return "WATCHDOG_DEADLINE_MISS";
        default: return "UNKNOWN_FAULT";
    }
}

}  // namespace astra
