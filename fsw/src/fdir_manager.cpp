#include "astra/fdir_manager.hpp"

#include <algorithm>

namespace astra {
namespace {

Mode response_mode(FaultResponse response, Mode current_mode) {
    switch (response) {
        case FaultResponse::NONE:
        case FaultResponse::HOLD_CURRENT_MODE:
            return current_mode;
        case FaultResponse::ENTER_DEGRADED_SENSOR:
            return Mode::DEGRADED_SENSOR;
        case FaultResponse::ENTER_DEGRADED_PAYLOAD:
            return Mode::DEGRADED_PAYLOAD;
        case FaultResponse::ENTER_SAFE:
            return Mode::SAFE;
    }

    return Mode::SAFE;
}

}  // namespace

FaultDisposition FdirManager::disposition_for(FaultCode fault) const {
    switch (fault) {
        case FaultCode::NONE:
            return {fault, FaultSeverity::NONE, FaultResponse::NONE, 0, false, false, "none", "none"};

        case FaultCode::SENSOR_TIMEOUT:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 90, true, true, "health monitor sensor age", "operator clear after valid sensor dwell"};

        case FaultCode::SENSOR_INVALID_DATA:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 85, true, true, "sensor validity monitor", "operator clear after valid sensor dwell"};

        case FaultCode::PAYLOAD_HEARTBEAT_TIMEOUT:
            return {fault, FaultSeverity::DEGRADED, FaultResponse::ENTER_DEGRADED_PAYLOAD, 50, true, false, "payload heartbeat monitor", "clear after heartbeat recovery dwell"};

        case FaultCode::CPU_OVERLOAD:
            return {fault, FaultSeverity::DEGRADED, FaultResponse::ENTER_DEGRADED_PAYLOAD, 60, true, false, "resource monitor", "clear after CPU recovery dwell"};

        case FaultCode::MEMORY_OVERLOAD:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 80, true, true, "resource monitor", "operator clear after memory recovery dwell"};

        case FaultCode::TELEMETRY_SOCKET_FAILURE:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 75, true, true, "telemetry transport", "operator clear after transport recovery"};

        case FaultCode::COMMAND_BAD_CRC:
            return {fault, FaultSeverity::ADVISORY, FaultResponse::HOLD_CURRENT_MODE, 20, false, false, "command decoder", "automatic after valid packet"};

        case FaultCode::COMMAND_UNKNOWN_ID:
            return {fault, FaultSeverity::ADVISORY, FaultResponse::HOLD_CURRENT_MODE, 15, false, false, "command decoder", "automatic after valid command"};

        case FaultCode::COMMAND_TIMEOUT:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 70, true, true, "command-link monitor", "operator clear after link recovery"};

        case FaultCode::WATCHDOG_DEADLINE_MISS:
            return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 100, true, true, "watchdog", "operator clear after timing investigation"};
    }

    return {fault, FaultSeverity::CRITICAL, FaultResponse::ENTER_SAFE, 255, true, true, "unknown", "operator review required"};
}

FaultCode FdirManager::select_primary_fault(
    const std::vector<FaultCode>& active_faults
) const {
    FaultCode selected = FaultCode::NONE;
    FaultDisposition selected_disposition = disposition_for(selected);

    for (const auto fault : active_faults) {
        if (fault == FaultCode::NONE) {
            continue;
        }

        const auto candidate = disposition_for(fault);
        const bool higher_priority = candidate.priority > selected_disposition.priority;
        const bool deterministic_tie_break =
            candidate.priority == selected_disposition.priority &&
            static_cast<std::uint16_t>(candidate.fault) <
                static_cast<std::uint16_t>(selected_disposition.fault);

        if (higher_priority || deterministic_tie_break) {
            selected = fault;
            selected_disposition = candidate;
        }
    }

    return selected;
}

FdirActionResult FdirManager::apply_fault(
    ModeManager& mode_manager,
    FaultCode fault
) const {
    FdirActionResult result;
    result.disposition = disposition_for(fault);
    result.mode_before = mode_manager.current_mode();
    result.requested_mode = response_mode(result.disposition.response, result.mode_before);
    result.mode_after = result.mode_before;

    if (result.disposition.response == FaultResponse::NONE ||
        result.disposition.response == FaultResponse::HOLD_CURRENT_MODE) {
        return result;
    }

    result.transition_attempted = true;
    result.transition_succeeded = mode_manager.transition_to(result.requested_mode, fault);

    if (!result.transition_succeeded && result.requested_mode != Mode::SAFE) {
        result.safe_fallback_used = true;
        result.requested_mode = Mode::SAFE;
        result.transition_succeeded = mode_manager.transition_to(Mode::SAFE, fault);
    }

    result.mode_after = mode_manager.current_mode();
    return result;
}

std::string fault_severity_to_string(FaultSeverity severity) {
    switch (severity) {
        case FaultSeverity::NONE:
            return "NONE";
        case FaultSeverity::ADVISORY:
            return "ADVISORY";
        case FaultSeverity::DEGRADED:
            return "DEGRADED";
        case FaultSeverity::CRITICAL:
            return "CRITICAL";
    }

    return "UNKNOWN_SEVERITY";
}

std::string fault_response_to_string(FaultResponse response) {
    switch (response) {
        case FaultResponse::NONE:
            return "NONE";
        case FaultResponse::HOLD_CURRENT_MODE:
            return "HOLD_CURRENT_MODE";
        case FaultResponse::ENTER_DEGRADED_SENSOR:
            return "ENTER_DEGRADED_SENSOR";
        case FaultResponse::ENTER_DEGRADED_PAYLOAD:
            return "ENTER_DEGRADED_PAYLOAD";
        case FaultResponse::ENTER_SAFE:
            return "ENTER_SAFE";
    }

    return "UNKNOWN_RESPONSE";
}

}  // namespace astra
